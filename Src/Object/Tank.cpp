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

	// ���f���̊�{�ݒ�

	// �y��---------------------------------------------------------

	// ���W
	transformBody_.pos = { 0.0f,50.0f,0.0f };

	// ���f������̊�{���
	transformBody_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BODY));

	// �傫��
	float scale = 0.3f;
	transformBody_.scl = { scale, scale, scale };

	// ���f������̊�{���X�V
	transformBody_.Update();

	// �E�ԗ�-------------------------------------------------------

	// ���f������̊�{���
	transformRWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// �傫��
	scale = 0.3f;
	transformRWheel_.scl = { scale, scale, scale };
	transformRWheel_.pos = VAdd(transformBody_.pos, { 40.0f,-30.0f,0.0f });

	// ���ԗ�-------------------------------------------------------

	// ���f������̊�{���
	transformLWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// �傫��
	scale = 0.3f;
	transformLWheel_.scl = { scale, scale, scale };
	transformLWheel_.pos = VAdd(transformBody_.pos, { -40.0f,-30.0f,0.0f });

	// �C�g----------------------------------------------------------

	// ���f������̊�{���
	transformBarrel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BARREL));

	// �傫��
	scale = 0.3f;
	transformBarrel_.scl = { scale, scale, scale };
	transformBarrel_.pos = VAdd(transformBody_.pos, { 0.0f,-10.0f,0.0f });

	// �y��
	MV1SetRotationXYZ(transformBody_.modelId, transformBody_.rot);
	MV1SetScale(transformBody_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformBody_.modelId, transformBody_.pos);

	// �E�ԗ�
	MV1SetRotationXYZ(transformRWheel_.modelId, transformRWheel_.rot);
	MV1SetScale(transformRWheel_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformRWheel_.modelId, transformRWheel_.pos);

	// ���ԗ�
	MV1SetRotationXYZ(transformLWheel_.modelId, transformLWheel_.rot);
	MV1SetScale(transformLWheel_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformLWheel_.modelId, transformLWheel_.pos);

	// �C�g
	MV1SetRotationXYZ(transformBarrel_.modelId, transformBarrel_.rot);
	MV1SetScale(transformBarrel_.modelId, { 0.3f,0.3f,0.3f });
	MV1SetPosition(transformBarrel_.modelId, transformBarrel_.pos);
	//Update();

}

void Tank::Update(void)
{

	// �ړ������ɉ�������]
	Rotate();



	// �e�̔���
	ProcessShot();

	// �d�͕����ɉ����ĉ�]������
	transformBody_.quaRot = grvMng_.GetTransform().quaRot;
	transformBody_.quaRot = transformBody_.quaRot.Mult(goalQuaRot_);
	transformRWheel_.quaRot = grvMng_.GetTransform().quaRot;
	transformRWheel_.quaRot = transformRWheel_.quaRot.Mult(transformBody_.quaRot);
	transformLWheel_.quaRot = grvMng_.GetTransform().quaRot;
	transformLWheel_.quaRot = transformLWheel_.quaRot.Mult(transformBody_.quaRot);
	transformBarrel_.quaRot = grvMng_.GetTransform().quaRot;
	transformBarrel_.quaRot = transformBarrel_.quaRot.Mult(transformBody_.quaRot);

	// �ړ�
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

	// ���ݐݒ肳��Ă����]�Ƃ̊p�x�������
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// �������l
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

	// ��]�̋��ʕ��
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);


}

void Tank::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	// �J�����̉�]���
	Quaternion cameraRot = SceneManager::GetInstance().GetCamera().lock()->GetQuaRotOutX();

	// �ړ��ʂ��[��
	VECTOR dir = AsoUtility::VECTOR_ZERO;

	// �X�s�[�h
	float speed = 10.0f;


	// ��]��YXZ�̏��Ԃōs��


	// �J���������ɑO�i������
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

		// �e(���)�Ƃ̑��΍��W���A�e(���)�̉�]���ɍ��킹�ĉ�]
		VECTOR localRotPos = transformBody_.quaRot.PosAxis({ 40.0f,-30.0f,0.0f });
		transformRWheel_.pos = VAdd(transformBody_.pos, localRotPos);
		transformRWheel_.quaRot = transformBody_.quaRot;

		// �e(���)�Ƃ̑��΍��W���A�e(���)�̉�]���ɍ��킹�ĉ�]
		localRotPos = transformBody_.quaRot.PosAxis({ -40.0f,-30.0f,0.0f });
		transformLWheel_.pos = VAdd(transformBody_.pos, localRotPos);
		transformLWheel_.quaRot = transformBody_.quaRot;

		// �e(���)�Ƃ̑��΍��W���A�e(���)�̉�]���ɍ��킹�ĉ�]
		localRotPos = transformBody_.quaRot.PosAxis({ 0.0f,-10.0f,0.0f });
		transformBarrel_.pos = VAdd(transformBody_.pos, localRotPos);
		transformBarrel_.quaRot = transformBody_.quaRot;

		SetGoalRotate(transformBody_.rot.y);

	}

	

}

void Tank::ProcessShot(void)
{
}
