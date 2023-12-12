#include "../Manager/InputManager.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/GravityManager.h"
#include "../Manager/Camera.h"
#include "../Object/ShotBase.h"
#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "Tank.h"

Tank::Tank(void)
{
	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;
}

Tank::~Tank(void)
{
}

void Tank::Init(void)
{

	// モデルの基本設定

	// 土台---------------------------------------------------------

	// 座標
	transformBody_.pos = { 0.0f,50.0f,0.0f };

	// ローカル回転
	localRotAddBody_ = { 0.0f,0.0f,0.0f };

	// モデル制御の基本情報
	transformBody_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BODY));

	// 大きさ
	float scale = 0.3f;
	transformBody_.scl = { scale, scale, scale };

	// モデル制御の基本情報更新
	transformBody_.Update();

	// 右車輪-------------------------------------------------------

	transformRWheel_.pos = VAdd(transformBody_.pos, { 40.0f,-30.0f,0.0f });

	// ローカル回転
	localRotAddRWheel_ = { 0.0f,0.0f,0.0f };

	// モデル制御の基本情報
	transformRWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// 大きさ
	scale = 0.3f;
	transformRWheel_.scl = { scale, scale, scale };

	// モデル制御の基本情報更新
	transformRWheel_.Update();

	// 左車輪-------------------------------------------------------

	transformLWheel_.pos = VAdd(transformBody_.pos, { -40.0f,-30.0f,0.0f });

	// ローカル回転
	localRotAddLWheel_ = { 0.0f,0.0f,0.0f };

	// モデル制御の基本情報
	transformLWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// 大きさ
	scale = 0.3f;
	transformLWheel_.scl = { scale, scale, scale };

	// モデル制御の基本情報更新
	transformLWheel_.Update();

	// 砲身----------------------------------------------------------

	transformBarrel_.pos = VAdd(transformBody_.pos, { 0.0f,-10.0f,0.0f });

	// ローカル回転
	localRotAddBarrel_ = { 0.0f,0.0f,0.0f };

	// モデル制御の基本情報
	transformBarrel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BARREL));

	// 大きさ
	scale = 0.3f;
	transformBarrel_.scl = { scale, scale, scale };

	// モデル制御の基本情報更新
	transformBarrel_.Update();

	movedPos_ = { 0.0f,0.0f,0.0f };

	movePow_ = {0.0f,0.0f,0.0f};

	jumpPow_ = { 0.0f,0.0f,0.0f };

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transformBody_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, -50.0f, 0.0f });
	capsule_->SetRadius(20.0f);

	//// 左車輪
	//MV1SetRotationXYZ(transformLWheel_.modelId, transformLWheel_.rot);
	//MV1SetScale(transformLWheel_.modelId, { 0.3f,0.3f,0.3f });
	//MV1SetPosition(transformLWheel_.modelId, transformLWheel_.pos);

	//// 砲身
	//MV1SetRotationXYZ(transformBarrel_.modelId, transformBarrel_.rot);
	//MV1SetScale(transformBarrel_.modelId, { 0.3f,0.3f,0.3f });
	//MV1SetPosition(transformBarrel_.modelId, transformBarrel_.pos);
	//Update();
	
}

void Tank::Update(void)
{

	// 重力による移動量
	//CalcGravityPow();



	// 移動
	ProcessMove();

	// 弾の発射
	ProcessShot();

	CalcSlope();

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定
	Collision();

	//SetGoalRotate(localRotAddBody_.y);
	//Rotate();
	transformBody_.quaRot = grvMng_.GetTransform().quaRot;
	transformBody_.quaRot = transformBody_.quaRot.Mult(Quaternion::Euler(0.0f,localRotAddBody_.y,0.0f));
	transform_.quaRot = grvMng_.GetTransform().quaRot;
	transform_.quaRot = transform_.quaRot.Mult(Quaternion::Euler(0.0f, localRotAddBody_.y, 0.0f));
	// 親(戦艦)との相対座標を、親(戦艦)の回転情報に合わせて回転
	//transformRWheel_.quaRot = transformRWheel_.quaRot.Mult(transformBody_.quaRot);
	//transformRWheel_.quaRot = transformBody_.quaRot;
	//transformRWheel_.quaRot = transformRWheel_.quaRot.Mult(Quaternion::Euler(localRotAddRWheel_));
	//VECTOR localRotPos = transformBody_.quaRot.PosAxis({ 40.0f,-30.0f,0.0f });
	//transformRWheel_.pos = VAdd(transformBody_.pos, localRotPos);

	//// 親(戦艦)との相対座標を、親(戦艦)の回転情報に合わせて回転
	//localRotPos = transformBody_.quaRot.PosAxis({ -40.0f,-30.0f,0.0f });
	//transformLWheel_.pos = VAdd(transformBody_.pos, localRotPos);
	//transformLWheel_.quaRot = transformBody_.quaRot;

	// 親(戦艦)との相対座標を、親(戦艦)の回転情報に合わせて回転
	VECTOR localRotPos = transformBody_.quaRot.PosAxis({ 0.0f,-10.0f,0.0f });
	transformBarrel_.pos = VAdd(transformBody_.pos, localRotPos);
	transformBarrel_.quaRot = transformBody_.quaRot;

	SyncParent(transformRWheel_, localRotAddRWheel_, { 40.0f,-30.0f,0.0f });
	SyncParent(transformLWheel_, localRotAddLWheel_, { -40.0f,-30.0f,0.0f });
	transformBody_.quaRot = transformBody_.quaRot.Mult(Quaternion::Euler(localRotAddBody_.x, 0.0f, 0.0f));
	transformBarrel_.quaRot = transformBody_.quaRot.Mult(Quaternion::Euler(localRotAddBody_.x, 0.0f, 0.0f));
	transformBarrel_.quaRot = transformBarrel_.quaRot.Mult(Quaternion::Euler(0.0f, 0.0f, localRotAddBarrel_.z));

	transformBody_.Update();
	transformRWheel_.Update();
	transformLWheel_.Update();
	transformBarrel_.Update();

	for (auto v : shots_)
	{
		v->Update();
	}

}

void Tank::Draw(void)
{
	MV1DrawModel(transformBody_.modelId);
	MV1DrawModel(transformRWheel_.modelId);
	MV1DrawModel(transformLWheel_.modelId);
	MV1DrawModel(transformBarrel_.modelId);

	DrawDebug();

	for (auto v : shots_)
	{
		v->Draw();
	}

}

void Tank::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.emplace_back(collider);
}

void Tank::ClearCollider(void)
{
	colliders_.clear();
}

void Tank::Collision(void)
{

	// 現在座標を起点に移動後座標を決める
	//movedPos_ = VAdd(transformBody_.pos, movePow_);
	transformBody_.pos = VAdd(transformBody_.pos, movePow_);

	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();

	// 移動
	//transformBody_.pos = movedPos_;

}

void Tank::CollisionGravity(void)
{


	// ジャンプ量を加算
	transformBody_.pos = VAdd(transformBody_.pos, jumpPow_);

	// 重力方向
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// 重力方向の反対
	VECTOR dirUpGravity = grvMng_.GetDirUpGravity();

	// 重力の強さ
	float gravityPow = grvMng_.GetPower();

	// モデルとの衝突判定を行うための線分(始点、終点)を作成
	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(VAdd(transformBody_.pos, {0.0f,100.0f,0.0f}), VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(VAdd(transformBody_.pos, { 0.0f,-100.0f,0.0f }), VScale(dirGravity, checkPow));
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
			transformBody_.pos = VAdd(hit.HitPosition, VScale(dirUpGravity, 70.0f));

			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
		}
	}

}

void Tank::CollisionCapsule(void)
{

	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = transformBody_.pos;
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
					transformBody_.pos = VAdd(transformBody_.pos, VScale(hit.Normal, 1.0f));
					// カプセルも一緒に移動させる
					trans.pos = transformBody_.pos;
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

void Tank::CalcGravityPow(void)
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

}

void Tank::CalcSlope(void)
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

	//// 傾斜による移動
	//if (AsoUtility::SqrMagnitude(jumpPow_) == 0.0f)
	//{
	//	float CHECK_ANGLE = 120.0f;
	//	if (slopeAngleDeg_ >= CHECK_ANGLE)
	//	{
	//		float diff = abs(slopeAngleDeg_ - CHECK_ANGLE);
	//		slopePow_ = VScale(slopeDir_, diff / 3.0f);
	//		movePow_ = VAdd(movePow_, slopePow_);
	//	}
	//}

}

void Tank::SetGoalRotate(double rotRad)
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

	//auto goal = Quaternion::Euler(0.0f, rotRad, 0.0f);
	//transformBody_.quaRot = Quaternion::Slerp(transformBody_.quaRot, goal, 0.1);

}

void Tank::Rotate(void)
{

	stepRotTime_ -= scnMng_.GetDeltaTime();

	// 回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);


}

void Tank::SyncParent(Transform& transform, VECTOR addAxis, VECTOR localPos)
{

	// 親(戦艦)の回転情報を取得
	Quaternion parentRot = transformBody_.quaRot;

#pragma region 補足
	// 砲台のローカル回転
	// Unity Z→X→Y = Y * X * Z
	//axis = Quaternion::AngleAxis(localRotAdd_.y, AsoUtility::AXIS_Y);
	//localRot = localRot.Mult(axis);

	//axis = Quaternion::AngleAxis(localRotAdd_.x, AsoUtility::AXIS_X);
	//localRot = localRot.Mult(axis);

	//axis = Quaternion::AngleAxis(localRotAdd_.z, AsoUtility::AXIS_Z);
	//localRot = localRot.Mult(axis);

#pragma endregion

	// 親(戦艦)回転からの相対回転
	Quaternion rot = Quaternion::Identity();
	//rot = rot.Mult(Quaternion::Euler({0.0f,0.0f,0.0f}));
	rot = rot.Mult(parentRot);

	// 稼働分の回転を加える
	rot = rot.Mult(Quaternion::Euler(addAxis));

	// 親(戦艦)の回転と相対回転を合成
	//transform.quaRot = parentRot.Mult(rot);
	transform.quaRot = rot;

	// 親(戦艦)との相対座標を、親(戦艦)の回転情報に合わせて回転
	VECTOR localRotPos = Quaternion::PosAxis(parentRot, localPos);

	// 相対座標をワールド座標に直す
	transform.pos = VAdd(transformBody_.pos, VScale(localRotPos, 1.0f));

	//// 重力方向に沿って回転させる
	//transform.quaRot = grvMng_.GetTransform().quaRot;

	//transform.quaRot = transform.quaRot.Mult(transformBody_.quaRot);

	// モデル制御の基本情報更新
	transform.Update();

}

void Tank::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	// カメラの回転情報
	Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();

	// 移動量をゼロ
	VECTOR dir = AsoUtility::VECTOR_ZERO;

	// スピード
	float speed = 10.0f;


	// 回転をYXZの順番で行う

	// 土台の回転
	if (ins.IsNew(KEY_INPUT_LEFT))
	{

		localRotAddBody_.y += AsoUtility::Deg2RadD(-1.0);
		localRotAddRWheel_.x += AsoUtility::Deg2RadD(1.0);
		localRotAddLWheel_.x += AsoUtility::Deg2RadD(-1.0);

	}

	if (ins.IsNew(KEY_INPUT_RIGHT))
	{

		localRotAddBody_.y += AsoUtility::Deg2RadD(1.0);
		localRotAddRWheel_.x += AsoUtility::Deg2RadD(-1.0);
		localRotAddLWheel_.x += AsoUtility::Deg2RadD(1.0);

	}

	//SyncParent(transformRWheel_, localRotAddRWheel_, { 40.0f,-30.0f,0.0f });
	//SyncParent(transformLWheel_, localRotAddLWheel_, { -40.0f,-30.0f,0.0f });

	// カメラ方向に前進したい
	if (ins.IsNew(KEY_INPUT_W))
	{

		dir = transform_.quaRot.GetForward();
		auto vec = VScale(dir, speed);
		transformBody_.pos = VAdd(transformBody_.pos, vec);
		localRotAddRWheel_.x += AsoUtility::Deg2RadD(1.0);
		localRotAddLWheel_.x += AsoUtility::Deg2RadD(1.0);

	}

	if (ins.IsNew(KEY_INPUT_S))
	{

		dir = transform_.quaRot.GetBack();
		auto vec = VScale(dir, speed);
		transformBody_.pos = VAdd(transformBody_.pos, vec);
		localRotAddRWheel_.x += AsoUtility::Deg2RadD(-1.0);
		localRotAddLWheel_.x += AsoUtility::Deg2RadD(-1.0);
	}
	
	auto deg = AsoUtility::Rad2DegF(localRotAddBody_.x);

	if (ins.IsNew(KEY_INPUT_UP) && deg >= -30.0)
	{
		localRotAddBody_.x += AsoUtility::Deg2RadD(-1.0);
	}

	if (ins.IsNew(KEY_INPUT_DOWN) && deg <= 15)
	{
		localRotAddBody_.x += AsoUtility::Deg2RadD(1.0);
	}

	localRotAddBarrel_.z += AsoUtility::Deg2RadD(1.0);

}

void Tank::DrawDebug(void)
{
	// 衝突
	DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0x000000);

	// カプセルコライダ
	capsule_->Draw();

}

void Tank::CreateShot(void)
{

	// 弾の生成フラグ
	bool isCreate = false;

	for (auto v : shots_)
	{
		if (v->GetState() == ShotBase::STATE::END)
		{
			// 以前に生成したインスタンスを使い回し
			v->Create({ transformBarrel_.pos.x,
				transformBarrel_.pos.y,
				transformBarrel_.pos.z, },
				transformBarrel_.GetForward());
			isCreate = true;
			break;
		}
	}
	if (!isCreate)
	{
		// 自機の前方方向
		auto dir = transformBarrel_.GetForward();
		// 新しいインスタンスを生成
		ShotBase* newShot = new ShotBase();
		newShot->Create({ transformBarrel_.pos.x,
			transformBarrel_.pos.y,
			transformBarrel_.pos.z },
			transformBarrel_.GetForward());

		// 弾の管理配列に追加
		shots_.push_back(newShot);
	}

}

void Tank::ProcessShot(void)
{

	auto& ins = InputManager::GetInstance();

	delayShot_ -= SceneManager::GetInstance().GetDeltaTime();
	if (delayShot_ <= 0.0f)
	{
		delayShot_ = 0.0f;
	}

	if (ins.IsNew(KEY_INPUT_N) && delayShot_ == 0.0f)
	{
		CreateShot();
		delayShot_ = TIME_DELAY_SHOT;
	}

}