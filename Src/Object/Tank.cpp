#include "../Manager/InputManager.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/GravityManager.h"
#include "../Manager/Camera.h"
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

	// 衝突判定
	Collision();

	// 移動
	ProcessMove();

	// 弾の発射
	ProcessShot();

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

}

void Tank::Draw(void)
{
	MV1DrawModel(transformBody_.modelId);
	MV1DrawModel(transformRWheel_.modelId);
	MV1DrawModel(transformLWheel_.modelId);
	MV1DrawModel(transformBarrel_.modelId);

	DrawDebug();
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

	// 衝突(重力)
	CollisionGravity();

}

void Tank::CollisionGravity(void)
{

	// 重力方向
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// 重力方向の反対
	VECTOR dirUpGravity = grvMng_.GetDirUpGravity();

	// 重力の強さ
	float gravityPow = grvMng_.GetPower();

	// モデルとの衝突判定を行うための線分(始点、終点)を作成
	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(transformBody_.pos, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(transformBody_.pos, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);
		//if (hit.HitFlag > 0)
		if (hit.HitFlag > 0)
		{

			// 傾斜計算用に衝突判定を保存しておく
			hitNormal_ = hit.Normal;
			hitPos_ = hit.HitPosition;

			// 衝突地点から、少し上に移動
			transformBody_.pos = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));
		}
	}
}

void Tank::CalcGravityPow(void)
{

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

void Tank::ProcessShot(void)
{
}

void Tank::DrawDebug(void)
{
	// 衝突
	DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0x000000);
}
