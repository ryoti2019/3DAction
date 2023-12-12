#include <EffekseerForDXLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Resource.h"
#include "ShotBase.h"

ShotBase::ShotBase(void)
{

	// 初期化
	stepAlive_ = 0.0f;
	dir_ = AsoUtility::VECTOR_ZERO;

	// モデル制御の基本情報
	ResourceManager& rem = ResourceManager::GetInstance();
	transform_.modelId =
		rem.LoadModelDuplicate(ResourceManager::SRC::SHOT_MODEL);
	float scale = 1.0f;
	transform_.scl = { scale, scale, scale };
	transform_.pos = AsoUtility::VECTOR_ZERO;
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler(AsoUtility::Deg2RadF(90.0f), 0.0f, 0.0f);

	transform_.Update();

	// 爆発エフェクト
	effectBlastResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::SHOT_EXPLOSION).handleId_;

}

ShotBase::~ShotBase(void)
{
}

void ShotBase::Create(VECTOR birthPos, VECTOR dir)
{

	// パラメータ設定
	SetParam();

	// 再利用可能なようにする
	// 指定方向に弾を飛ばす
	dir_ = dir;

	// 重力
	gravityPow_ = 0.0f;

	// 弾の発生位置
	transform_.pos = birthPos;

	// 弾モデルの向き(角度)を指定方向に合わせる
	transform_.quaRot = Quaternion::LookRotation(dir_);

	// 生存フラグ、時間の初期化
	stepAlive_ = timeAlive_;

	// モデル制御の基本情報更新
	transform_.Update();

	// 状態遷移
	ChangeState(STATE::SHOT);

}

void ShotBase::Blast(void)
{

	ChangeState(STATE::BLAST);

}

ShotBase::STATE ShotBase::GetState(void)
{
	return state_;
}

void ShotBase::Update(void)
{

	switch (state_)
	{
	case ShotBase::STATE::NONE:
		break;
	case ShotBase::STATE::SHOT:
		UpdateShot();
		break;
	case ShotBase::STATE::BLAST:
		UpdateBlast();
		break;
	case ShotBase::STATE::END:
		break;
	}

	// モデル制御の基本情報更新
	transform_.Update();

}

void ShotBase::CheckAlive(void)
{

	// 生存時間を減らす
	stepAlive_ -= SceneManager::GetInstance().GetDeltaTime();
	if (stepAlive_ < 0.0f)
	{
		ChangeState(STATE::END);
	}

}

void ShotBase::Draw(void)
{

	switch (state_)
	{
	case ShotBase::STATE::NONE:
		break;
	case ShotBase::STATE::SHOT:
		MV1DrawModel(transform_.modelId);
		break;
	case ShotBase::STATE::BLAST:
		break;
	case ShotBase::STATE::END:
		break;
	}

}

void ShotBase::Release(void)
{
	MV1DeleteModel(transform_.modelId);
}

VECTOR ShotBase::GetPos(void) const
{
	return transform_.pos;
}

void ShotBase::SetPos(VECTOR pos)
{
	transform_.pos = pos;
}

float ShotBase::GetSpeed(void) const
{
	return speed_;
}

float ShotBase::GetTimeAlive(void) const
{
	return timeAlive_;
}

float ShotBase::GetCollisionRadius(void) const
{
	return collisionRadius_;
}

void ShotBase::Move(void)
{
	// 更に加速度的に重力を加える
	gravityPow_ += 9.81f / 60.0f;

	// 移動
	VECTOR velocity = VScale(dir_, speed_);
	transform_.pos = VAdd(transform_.pos, velocity);
	transform_.pos = VAdd(transform_.pos, VScale({ 0.0f,-1.0f,0.0f }, gravityPow_));


}

void ShotBase::SetParam(void)
{

	// 弾の速度
	speed_ = 30.0f;

	// 生存時間
	timeAlive_ = 5.0f;

	// 衝突用球体半径
	collisionRadius_ = 10.0f;

}

void ShotBase::UpdateShot(void)
{

	// 生存チェック
	CheckAlive();

	if (state_ != STATE::SHOT)
	{
		
		// 処理中断
		return;

	}

	// 移動処理
	Move();

}

void ShotBase::UpdateBlast(void)
{

	// エフェクト再生チェック
	// 0:再生中、-1:再生されていない、もしくは再生終了
	int ret = IsEffekseer3DEffectPlaying(effectBlastPlayId_);
	if (ret == -1)
	{
		// 終了状態へ
		ChangeState(STATE::END);
	}

}

void ShotBase::ChangeState(STATE state)
{
	if (state_ == state)
	{
		// 同じ状態なら処理しない
		return;
	}
	// 状態更新
	state_ = state;
	switch (state)
	{
	case ShotBase::STATE::NONE:
		break;
	case ShotBase::STATE::SHOT:
		break;
	case ShotBase::STATE::BLAST:
		// 爆発エフェクトを再生する
		PlayBlastEffect();
		break;
	case ShotBase::STATE::END:
		break;
	}

}

void ShotBase::PlayBlastEffect(void)
{
	// 爆発エフェクトを再生する
	effectBlastPlayId_ = PlayEffekseer3DEffect(effectBlastResId_);
	SetScalePlayingEffekseer3DEffect(
		effectBlastPlayId_, BLAST_SCALE, BLAST_SCALE, BLAST_SCALE);
	SetRotationPlayingEffekseer3DEffect(effectBlastPlayId_, 0.0f, 0.0f, 0.0f);
	VECTOR pos = transform_.pos;
	SetPosPlayingEffekseer3DEffect(effectBlastPlayId_, pos.x, pos.y, pos.z);
}
