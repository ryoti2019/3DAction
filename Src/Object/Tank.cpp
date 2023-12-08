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

	// モデル制御の基本情報
	transformBody_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BODY));

	// 大きさ
	float scale = 0.3f;
	transformBody_.scl = { scale, scale, scale };

	// モデル制御の基本情報更新
	transformBody_.Update();

	// 右車輪-------------------------------------------------------

	// モデル制御の基本情報
	transformRWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// 大きさ
	scale = 0.3f;
	transformRWheel_.scl = { scale, scale, scale };
	transformRWheel_.pos = VAdd(transformBody_.pos, { 40.0f,-30.0f,0.0f });

	// 左車輪-------------------------------------------------------

	// モデル制御の基本情報
	transformLWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// 大きさ
	scale = 0.3f;
	transformLWheel_.scl = { scale, scale, scale };
	transformLWheel_.pos = VAdd(transformBody_.pos, { -40.0f,-30.0f,0.0f });

	// 砲身----------------------------------------------------------

	// モデル制御の基本情報
	transformBarrel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BARREL));

	// 大きさ
	scale = 0.3f;
	transformBarrel_.scl = { scale, scale, scale };
	transformBarrel_.pos = VAdd(transformBody_.pos, { 0.0f,-10.0f,0.0f });

	// 土台
	MV1SetRotationXYZ(transformBody_.modelId, transformBody_.rot);
	MV1SetScale(transformBody_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformBody_.modelId, transformBody_.pos);

	// 右車輪
	MV1SetRotationXYZ(transformRWheel_.modelId, transformRWheel_.rot);
	MV1SetScale(transformRWheel_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformRWheel_.modelId, transformRWheel_.pos);

	// 左車輪
	MV1SetRotationXYZ(transformLWheel_.modelId, transformLWheel_.rot);
	MV1SetScale(transformLWheel_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformLWheel_.modelId, transformLWheel_.pos);

	// 砲身
	MV1SetRotationXYZ(transformBarrel_.modelId, transformBarrel_.rot);
	MV1SetScale(transformBarrel_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformBarrel_.modelId, transformBarrel_.pos);
	//Update();

}

void Tank::Update(void)
{

	// 移動方向に応じた回転
	Rotate();



	// 弾の発射
	ProcessShot();

	// 重力方向に沿って回転させる
	transformBody_.quaRot = grvMng_.GetTransform().quaRot;
	transformBody_.quaRot = transformBody_.quaRot.Mult(goalQuaRot_);
	transformRWheel_.quaRot = grvMng_.GetTransform().quaRot;
	transformRWheel_.quaRot = transformRWheel_.quaRot.Mult(transformBody_.quaRot);
	transformLWheel_.quaRot = grvMng_.GetTransform().quaRot;
	transformLWheel_.quaRot = transformLWheel_.quaRot.Mult(transformBody_.quaRot);
	transformBarrel_.quaRot = grvMng_.GetTransform().quaRot;
	transformBarrel_.quaRot = transformBarrel_.quaRot.Mult(transformBody_.quaRot);

	// 移動
	ProcessMove();

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
}

void Tank::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.emplace_back(collider);
}

void Tank::ClearCollider(void)
{
	colliders_.clear();
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


	// カメラ方向に前進したい
	if (ins.IsNew(KEY_INPUT_W))
	{
		dir = transformBody_.quaRot.GetForward();
		Quaternion rotPow = Quaternion::Identity();
		rotPow = Quaternion::Mult(
			rotPow,
			Quaternion::AngleAxis(AsoUtility::Deg2RadF(10), AsoUtility::AXIS_X));
		transformRWheel_.quaRot = Quaternion::Mult(transformRWheel_.quaRot, rotPow);
	}
	if (ins.IsNew(KEY_INPUT_A))
	{
		transformBody_.rot.y += AsoUtility::Deg2RadD(-1.0);
	}
	if (ins.IsNew(KEY_INPUT_S))
	{
		dir = transformBody_.quaRot.GetBack();
	}
	if (ins.IsNew(KEY_INPUT_D))
	{
		transformBody_.rot.y += AsoUtility::Deg2RadD(1.0);
	}

	if (!AsoUtility::EqualsVZero(transformBody_.rot) || !AsoUtility::EqualsVZero(dir))
	{
		auto vec = VScale(dir, speed);
		transformBody_.pos = VAdd(transformBody_.pos, vec);

		// 親(戦艦)との相対座標を、親(戦艦)の回転情報に合わせて回転
		VECTOR localRotPos = transformBody_.quaRot.PosAxis({ 40.0f,-30.0f,0.0f });
		transformRWheel_.pos = VAdd(transformBody_.pos, localRotPos);
		transformRWheel_.quaRot = transformBody_.quaRot;

		// 親(戦艦)との相対座標を、親(戦艦)の回転情報に合わせて回転
		localRotPos = transformBody_.quaRot.PosAxis({ -40.0f,-30.0f,0.0f });
		transformLWheel_.pos = VAdd(transformBody_.pos, localRotPos);
		transformLWheel_.quaRot = transformBody_.quaRot;

		// 親(戦艦)との相対座標を、親(戦艦)の回転情報に合わせて回転
		localRotPos = transformBody_.quaRot.PosAxis({ 0.0f,-10.0f,0.0f });
		transformBarrel_.pos = VAdd(transformBody_.pos, localRotPos);
		transformBarrel_.quaRot = transformBody_.quaRot;

		SetGoalRotate(transformBody_.rot.y);

	}

	

}

void Tank::ProcessShot(void)
{
}
