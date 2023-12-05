#include <string>
#include <EffekseerForDXLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/GravityManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "Common/AnimationController.h"
#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "Common/SpeechBalloon.h"
#include "Planet.h"
#include "Player.h"

Player::Player(void)
{

	animationController_ = nullptr;
	state_ = STATE::NONE;

	jumpPow_ = AsoUtility::VECTOR_ZERO;

	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	isJump_ = false;
	imgShadow_ = -1;

	reserveStartPos_ = AsoUtility::VECTOR_ZERO;
	stepWarp_ = 0.0f;
	timeWarp_ = 0.0f;
	warpReservePos_ = AsoUtility::VECTOR_ZERO;

	preWarpName_ = Stage::NAME::MAIN_PLANET;

	// 状態管理
	stateChanges_.emplace(STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));

	stateChanges_.emplace(
		STATE::WARP_RESERVE, std::bind(&Player::ChangeStateWarpReserve, this));

	stateChanges_.emplace(
		STATE::WARP_MOVE, std::bind(&Player::ChangeStateWarpMove, this));

}

Player::~Player(void)
{
}

void Player::Init(void)
{

	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	transform_.scl = AsoUtility::VECTOR_ONE;
	transform_.pos = { 0.0f, -30.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();

	// アニメーションの設定
	InitAnimation();

	// 丸影画像
	imgShadow_ = resMng_.Load(
		ResourceManager::SRC::PLAYER_SHADOW).handleId_;

	// 足煙
	effectSmokeResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FOOT_SMOKE).handleId_;

	// 左手のエフェクト
	effectHandLResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::WARP_ORBIT).handleId_;

	// 右手のエフェクト
	effectHandRResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::WARP_ORBIT).handleId_;

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 30.0f, 0.0f });
	capsule_->SetRadius(20.0f);

	// 初期状態
	ChangeState(STATE::PLAY);

}

void Player::Update(void)
{

	// 更新ステップ
	stateUpdate_();

	transform_.Update();

	// アニメーションの更新
	animationController_->Update();
	
}

void Player::Draw(void)
{

	// モデルの描画
	MV1DrawModel(transform_.modelId);

	// 丸影描画
	DrawShadow();

	// デバッグ用描画
	DrawDebug();

}

void Player::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.emplace_back(collider);
}

void Player::ClearCollider(void)
{
	colliders_.clear();
}

const Capsule& Player::GetCapsule(void) const
{
	return *capsule_;
}

void Player::StartWarpReserve(float time, const Quaternion& goalRot, const VECTOR& goalPos)
{

	// ワープ準備時間
	timeWarp_ = time;

	// ワープ準備経過時間
	stepWarp_ = time;

	// ワープ準備完了時の回転
	warpQua_ = goalRot;

	// ワープ準備完了時の座標
	warpReservePos_ = goalPos;

	// ワープ前の惑星情報を保持
	preWarpName_ = grvMng_.GetActivePlanet().lock()->GetName();

	ChangeState(STATE::WARP_RESERVE);

}

void Player::CollisionCapsule(void)
{

	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);

	// カプセルとの衝突判定
	for (const auto c : colliders_)
	{

		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());

		// 衝突した複数のポリゴンと衝突回避するまで、
		// プレイヤーの位置を移動させる
		for (int i = 0; i < hits.HitNum; i++)
		{

			auto hit = hits.Dim[i];

			// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
			// この時、移動させる方向は、移動前座標に向いた方向であったり、
			// 衝突したポリゴンの法線方向だったりする
			for (int tryCnt = 0; tryCnt < 10; tryCnt++)
			{

				// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
				// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{

					// 法線の方向にちょっとだけ移動させる
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
					// カプセルも一緒に移動させる
					trans.pos = movedPos_;
					trans.Update();
					continue;

				}
				break;
			}
		}

		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);

	}
}

void Player::InitAnimation(void)
{

	std::string path = Application::PATH_MODEL + "Player/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Idle.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + "FastRun.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::JUMP, path + "Jump.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::WARP_PAUSE, path + "WarpPose.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::FLY, path + "Flying.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::FALLING, path + "Falling.mv1", 80.0f);
	animationController_->Add((int)ANIM_TYPE::VICTORY, path + "Victory.mv1", 60.0f);

	animationController_->Play((int)ANIM_TYPE::IDLE);

}

void Player::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();

}

void Player::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Player::UpdateNone, this);
}

void Player::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&Player::UpdatePlay, this);
}

void Player::ChangeStateWarpReserve(void)
{

	stateUpdate_ = std::bind(&Player::UpdateWarpReserve, this);
	jumpPow_ = AsoUtility::VECTOR_ZERO;

	// ワープ準備開始時のプレイヤー情報
	reserveStartQua_ = transform_.quaRot;
	reserveStartPos_ = transform_.pos;

	animationController_->Play((int)Player::ANIM_TYPE::WARP_PAUSE);

}

void Player::ChangeStateWarpMove(void)
{

	stateUpdate_ = std::bind(&Player::UpdateWarpMove, this);

	animationController_->Play((int)Player::ANIM_TYPE::FLY);

	EffectHand();

}

void Player::UpdateNone(void)
{
}

void Player::UpdatePlay(void)
{

	// 移動処理
	ProcessMove();

	// 移動方向に応じた回転
	Rotate();

	// ジャンプ処理
	ProcessJump();

	CalcSlope();

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定
	Collision();

	// 重力方向に沿って回転させる
	transform_.quaRot = grvMng_.GetTransform().quaRot;
	transform_.quaRot = transform_.quaRot.Mult(playerRotY_);

	// 歩きエフェクト
	EffectFootSmoke();

}

void Player::UpdateWarpReserve(void)
{

	stepWarp_ -= SceneManager::GetInstance().GetDeltaTime();

	if (stepWarp_ < 0.0f)
	{

		// ワープ準備が完了(2秒経過)
		transform_.quaRot = warpQua_;
		transform_.pos = warpReservePos_;

		ChangeState(STATE::WARP_MOVE);

	}
	else
	{

		// ワープ準備中で、回転と座標を補間
		// 線形補間を行う
		transform_.pos = AsoUtility::Lerp(reserveStartPos_, warpReservePos_, 1.0f - (stepWarp_ / timeWarp_));

		// 球面補間を行う
		transform_.quaRot = Quaternion::Slerp(reserveStartQua_, warpQua_, 1.0f - (stepWarp_ / timeWarp_));

	}

}

void Player::UpdateWarpMove(void)
{

	// 移動処理→座標＋移動量
	// 移動量　＝方向×スピード

	transform_.pos = VAdd(transform_.pos, VScale(transform_.GetForward(), 20.0f));
	transform_.Update();
	
	SyncHandEffect();

	// 次の惑星に切り替わったらワープ状態からプレイ状態へ切り替える
	Stage::NAME name = grvMng_.GetActivePlanet().lock()->GetName();
	if (name != preWarpName_)
	{
		// 落下アニメーション
		animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
		animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);
		ChangeState(Player::STATE::PLAY);
		StopEffekseer3DEffect(effectHandLPlayId_);
		StopEffekseer3DEffect(effectHandRPlayId_);
		return;
	}

}

void Player::DrawDebug(void)
{

	int white = 0xffffff;
	int black = 0x000000;
	int red = 0xff0000;
	int green = 0x00ff00;
	int blue = 0x0000ff;
	int yellow = 0xffff00;
	int purpl = 0x800080;

	VECTOR v;

	// キャラ基本情報
	//-------------------------------------------------------
	// キャラ座標
	v = transform_.pos;
	DrawFormatString(20, 60, black, "キャラ座標 ： (%0.2f, %0.2f, %0.2f)",
		v.x, v.y, v.z
	);
	//-------------------------------------------------------

	//// 衝突
	//DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0x000000);

	//// カプセルコライダ
	//capsule_->Draw();

	// アクティブな惑星
	DrawFormatString(20, 80, black, "惑星　　　 ： %d",
		(int)grvMng_.GetActivePlanet().lock()->GetName()
	);

}

void Player::ProcessMove(void)
{

	auto& ins = InputManager::GetInstance();

	// カメラの回転情報
	Quaternion cameraRot = mainCamera->GetQuaRotOutX();

	// 移動量をゼロ
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;
	// 回転したい角度
	double rotRad = 0;

	// カメラ方向に前進したい
	if (ins.IsNew(KEY_INPUT_W))
	{
		rotRad = AsoUtility::Deg2RadD(0.0);
		dir = cameraRot.GetForward();
	}
	if (ins.IsNew(KEY_INPUT_A))
	{
		rotRad = AsoUtility::Deg2RadD(-90.0);
		dir = cameraRot.GetLeft();
	}
	if (ins.IsNew(KEY_INPUT_S))
	{
		rotRad = AsoUtility::Deg2RadD(180.0);
		dir = cameraRot.GetBack();
	}
	if (ins.IsNew(KEY_INPUT_D))
	{
		rotRad = AsoUtility::Deg2RadD(90.0);
		dir = cameraRot.GetRight();
	}

	if (!AsoUtility::EqualsVZero(dir) && (isJump_ || IsEndLanding()))
	{

		// 移動処理
		speed_ = SPEED_MOVE;
		if (ins.IsNew(KEY_INPUT_RSHIFT))
		{
			speed_ = SPEED_RUN;
		}
		moveDir_ = dir;
		movePow_ = VScale(dir, speed_);

		// 回転処理
		SetGoalRotate(rotRad);

		if (!isJump_ && IsEndLanding())
		{
			// アニメーション
			if (ins.IsNew(KEY_INPUT_RSHIFT))
			{
				animationController_->Play((int)ANIM_TYPE::FAST_RUN);
			}
			else
			{
				animationController_->Play((int)ANIM_TYPE::RUN);
			}
		}

	}
	else
	{
		if (!isJump_ && IsEndLanding())
		{
			animationController_->Play((int)ANIM_TYPE::IDLE);
		}
	}

}

void Player::SetGoalRotate(double rotRad)
{

	VECTOR cameraRot = mainCamera->GetAngles();

	Quaternion axis =
		Quaternion::AngleAxis(
			(double)cameraRot.y + rotRad, AsoUtility::AXIS_Y);

	// 現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// しきい値
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}

	goalQuaRot_ = axis;

}

void Player::Rotate(void)
{

	stepRotTime_ -= scnMng_.GetDeltaTime();

	// 回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

}

void Player::Collision(void)
{

	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// 衝突(カプセル)
	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();

	// 移動
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;

}

void Player::CollisionGravity(void)
{

	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// 重力方向の反対
	VECTOR dirUpGravity = grvMng_.GetDirUpGravity();

	// 重力の強さ
	float gravityPow = grvMng_.GetPower();

	// モデルとの衝突判定を行うための線分(始点、終点)を作成
	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);
		//if (hit.HitFlag > 0)
		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{

			// 傾斜計算用に衝突判定を保存しておく
			hitNormal_ = hit.Normal;
			hitPos_ = hit.HitPosition;

			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));
			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
			stepJump_ = 0.0f;
			if (isJump_)
			{
				// 着地モーション
				animationController_->Play(
					(int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);
			}
			isJump_ = false;
		}
	}

}

void Player::CalcGravityPow(void)
{

	// 重力方向
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// 重力の強さ
	float gravityPow = grvMng_.GetPower();

	// 重力
	// 重力を作る
	VECTOR gravity = VScale(dirGravity, gravityPow);
	// メンバ変数 jumpPow_ に重力計算を行う(加速度)
	jumpPow_ = VAdd(jumpPow_, gravity);

	// 内積
	float dot = VDot(dirGravity, jumpPow_);
	if (dot >= 0.0f)
	{
		// 重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
		jumpPow_ = gravity;
	}

}

void Player::CalcSlope(void)
{

	VECTOR gravityUp = grvMng_.GetDirUpGravity();

	// 重力の反対方向から地面の法線方向に向けた回転量を取得
	Quaternion up2GNorQua = Quaternion::FromToRotation(gravityUp, hitNormal_);

	// 取得した回転の軸と角度を取得する
	float angle = 0.0f;
	float* anglePtr = &angle;
	VECTOR axis;
	up2GNorQua.ToAngleAxis(anglePtr, &axis);

	// 90度足して、傾斜ベクトルへの回転を取得する
	Quaternion slopeQ = Quaternion::AngleAxis(
		angle + AsoUtility::Deg2RadD(90.0), axis);

	// 地面の傾斜線(黄色)
	slopeDir_ = slopeQ.PosAxis(gravityUp);

	// 傾斜の角度
	slopeAngleDeg_ = static_cast<float>(
		AsoUtility::AngleDeg(gravityUp, slopeDir_));

	// 傾斜による移動
	if (AsoUtility::SqrMagnitude(jumpPow_) == 0.0f)
	{
		float CHECK_ANGLE = 120.0f;
		if (slopeAngleDeg_ >= CHECK_ANGLE)
		{
			float diff = abs(slopeAngleDeg_ - CHECK_ANGLE);
			slopePow_ = VScale(slopeDir_, diff / 3.0f);
			movePow_ = VAdd(movePow_, slopePow_);
		}
	}

}

void Player::ProcessJump(void)
{

	bool isHit = CheckHitKey(KEY_INPUT_BACKSLASH);

	// ジャンプ
	if (isHit && (isJump_ || IsEndLanding()))
	{
		if (!isJump_)
		{
			// 制御無しジャンプ
			//animationController_->Play((int)ANIM_TYPE::JUMP);

			// ループしないジャンプ
			//animationController_->Play((int)ANIM_TYPE::JUMP,false);

			// 切り取りジャンプ
			//animationController_->Play((int)ANIM_TYPE::JUMP, false, 13.0f, 24.0f);

			// 無理やりアニメーション
			animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
			animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);

		}
		isJump_ = true;
		// ジャンプの入力受付時間をヘラス
		stepJump_ += scnMng_.GetDeltaTime();
		if (stepJump_ < TIME_JUMP_IN)
		{
			 jumpPow_ = VScale(grvMng_.GetDirUpGravity(), POW_JUMP);
		}
	}

	// ボタンを離したらジャンプ力に加算しない
	if (!isHit)
	{
		stepJump_ = TIME_JUMP_IN;
	}

}

bool Player::IsEndLanding(void)
{

	bool ret = true;

	// アニメーションがジャンプではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::JUMP)
	{
		return ret;
	}

	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;
	}

	return false;

}

bool Player::IsWarpMove(void)const
{
	return state_ == STATE::PLAY;
}

bool Player::IsPlay(void)const
{
	return state_ == STATE::WARP_MOVE;
}

void Player::DrawShadow(void)
{

	// プレイヤーの影を描画
	int i, j;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3];
	VECTOR SlideVec;
	int ModelHandle;

	// ライティングを無効にする
	SetUseLighting(FALSE);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// 影を落とすモデルの数だけ繰り返し
	for (auto c : colliders_)
	{

		// モデルのハンドルID
		ModelHandle = c.lock()->modelId_;

		// プレイヤーの直下に存在する地面のポリゴンを取得
		HitResDim = MV1CollCheck_Capsule(ModelHandle, -1, transform_.pos, VAdd(transform_.pos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f)), PLAYER_SHADOW_SIZE);

		// 頂点データで変化が無い部分をセット
		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
		Vertex[0].su = 0.0f;
		Vertex[0].sv = 0.0f;
		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		// 球の直下に存在するポリゴンの数だけ繰り返し
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			// ポリゴンの座標は地面ポリゴンの座標
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			// ちょっと持ち上げて重ならないようにする
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			// ポリゴンの不透明度を設定する
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[0].dif.a = 128 * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[1].dif.a = 128 * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[2].dif.a = 128 * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			// ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;

			// 影ポリゴンを描画
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(HitResDim);
	}
	// ライティングを有効にする
	SetUseLighting(TRUE);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);

}

void Player::EffectFootSmoke(void)
{

	// エフェクト再生
	effectSmokePlayId_ = PlayEffekseer3DEffect(effectSmokeResId_);

	// 大きさ
	float SCALE = 5.0f;

	if (stepFootSmoke_ >= 0.0f)
	{
		stepFootSmoke_ -= SceneManager::GetInstance().GetDeltaTime();
	}

	if (!AsoUtility::EqualsVZero(moveDiff_) && stepFootSmoke_ <= 0.0f && !isJump_)
	{
		SetScalePlayingEffekseer3DEffect(effectSmokePlayId_, SCALE, SCALE, SCALE);
		SetPosPlayingEffekseer3DEffect(effectSmokePlayId_, transform_.pos.x, transform_.pos.y, transform_.pos.z);
		stepFootSmoke_ = TERM_FOOT_SMOKE;
	}

}

void Player::EffectHand(void)
{

	// エフェクト再生
	effectHandLPlayId_ = PlayEffekseer3DEffect(effectHandLResId_);
	effectHandRPlayId_ = PlayEffekseer3DEffect(effectHandRResId_);

}

void Player::SyncHandEffect(void)
{

	// 座標
	auto leftPos = MV1GetFramePosition(transform_.modelId, 11);
	auto rightPos = MV1GetFramePosition(transform_.modelId, 35);

	// 大きさ
	float SCALE = 10.0f;

	// 手から出るエフェクト
	// 左手
	SetScalePlayingEffekseer3DEffect(effectHandLPlayId_, SCALE, SCALE, SCALE);
	SetPosPlayingEffekseer3DEffect(effectHandLPlayId_, leftPos.x, leftPos.y, leftPos.z);
	SetRotationPlayingEffekseer3DEffect(effectHandLPlayId_, transform_.rot.x, transform_.rot.y, transform_.rot.z);

	// 右手
	SetScalePlayingEffekseer3DEffect(effectHandRPlayId_, SCALE, SCALE, SCALE);
	SetPosPlayingEffekseer3DEffect(effectHandRPlayId_, rightPos.x, rightPos.y, rightPos.z);
	SetRotationPlayingEffekseer3DEffect(effectHandRPlayId_, transform_.rot.x, transform_.rot.y, transform_.rot.z);

}
