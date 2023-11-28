#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "Common/Transform.h"
#include "Common/Capsule.h"
#include "Player.h"
#include "WarpStar.h"

WarpStar::WarpStar(Player& player, const Transform& transform) : player_(player) 
{

	transform_ = transform;

	state_ = STATE::NONE;

	// 状態管理
	stateChanges_.emplace(STATE::IDLE, std::bind(&WarpStar::ChangeStateIdle, this));
	stateChanges_.emplace(STATE::RESERVE, std::bind(&WarpStar::ChangeStateReserve, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&WarpStar::ChangeStateMove, this));

}

WarpStar::~WarpStar(void)
{
}

void WarpStar::Init(void)
{

	// モデルの基本情報
	transform_.SetModel(
		resMng_.LoadModelDuplicate(
			ResourceManager::SRC::WARP_STAR)
	);
	transform_.Update();

	// 回転エフェクト
	effectRotParticleResId_ = resMng_.Load(
		ResourceManager::SRC::WARP_STAR_ROT_EFF).handleId_;

	// Zが無回転の状態を保持しておく
	VECTOR angles = transform_.quaRot.ToEuler();
	warpQua_ = Quaternion::Euler(angles.x, angles.y, 0.0f);

	// ワープ開始座標(ワールド座標)
	warpStartPos_ =
		VAdd(transform_.pos, warpQua_.PosAxis(WARP_RELATIVE_POS));

	stepEffect_ = TERM_EFFECT;

	ChangeState(STATE::IDLE);

}

void WarpStar::Update(void)
{

	// 更新ステップ
	stateUpdate_();

}

void WarpStar::Draw(void)
{
	MV1DrawModel(transform_.modelId);
}

void WarpStar::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();

}

void WarpStar::ChangeStateIdle(void)
{
	stateUpdate_ = std::bind(&WarpStar::UpdateIdle, this);
}

void WarpStar::ChangeStateReserve(void)
{

	stateUpdate_ = std::bind(&WarpStar::UpdateReserve, this);

	// プレイヤーの状態を変更
	player_.StartWarpReserve(TIME_WARP_RESERVE, warpQua_, warpStartPos_);

}

void WarpStar::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&WarpStar::UpdateMove, this);
}

void WarpStar::UpdateIdle(void)
{

	// 回転
	RotateZ(SPEED_ROT_IDLE);

	// エフェクト
	PlayEffectRotParticle();

	auto player = player_.GetCapsule();
	auto vec = VSub(player.GetCenter(), transform_.pos);
	auto length = AsoUtility::Magnitude(vec);
	if (length < 100.0f)
	{
		ChangeState(STATE::RESERVE);
	}

}

void WarpStar::UpdateReserve(void)
{

	// 回転
	RotateZ(SPEED_ROT_RESERVE);

	// エフェクト
	PlayEffectRotParticle();

}

void WarpStar::UpdateMove(void)
{
}

void WarpStar::RotateZ(float speed)
{

	Quaternion axis =
		Quaternion::AngleAxis(
			AsoUtility::Deg2RadF(speed), AsoUtility::AXIS_Z);

	transform_.quaRot = transform_.quaRot.Mult(axis);

	transform_.Update();

}

void WarpStar::PlayEffectRotParticle(void)
{

	// エフェクト再生
	effectRotParticlePlayId_ = PlayEffekseer3DEffect(effectRotParticleResId_);

	float SCALE = 5.0f;
	// 大きさ
	SetScalePlayingEffekseer3DEffect(effectRotParticlePlayId_, SCALE, SCALE, SCALE);

	// 位置
	SyncParticleEffect();

}

void WarpStar::SyncParticleEffect(void)
{

	// 追従対象の位置
	VECTOR followPos = transform_.pos;

	// 追従対象の向き
	Quaternion followRot = transform_.quaRot;

	VECTOR rot = Quaternion::ToEuler(followRot);

	// 追従対象から自機までの相対座標
	VECTOR effectPos = followRot.PosAxis(EFFECT_RELATIVE_POS);

	// エフェクトの位置の更新
	effectPos_ = VAdd(followPos, effectPos);

	// 位置の設定
	SetPosPlayingEffekseer3DEffect(effectRotParticlePlayId_, effectPos_.x, effectPos_.y, effectPos_.z);
	SetRotationPlayingEffekseer3DEffect(effectRotParticlePlayId_, rot.x, rot.y, rot.z);

	transform_.Update();

}
