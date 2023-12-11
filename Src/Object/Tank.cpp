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
	// �Փ˃`�F�b�N
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;
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

	// ���[�J����]
	localRotAddBody_ = { 0.0f,0.0f,0.0f };

	// ���f������̊�{���
	transformBody_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BODY));

	// �傫��
	float scale = 0.3f;
	transformBody_.scl = { scale, scale, scale };

	// ���f������̊�{���X�V
	transformBody_.Update();

	// �E�ԗ�-------------------------------------------------------

	transformRWheel_.pos = VAdd(transformBody_.pos, { 40.0f,-30.0f,0.0f });

	// ���[�J����]
	localRotAddRWheel_ = { 0.0f,0.0f,0.0f };

	// ���f������̊�{���
	transformRWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// �傫��
	scale = 0.3f;
	transformRWheel_.scl = { scale, scale, scale };

	// ���f������̊�{���X�V
	transformRWheel_.Update();

	// ���ԗ�-------------------------------------------------------

	transformLWheel_.pos = VAdd(transformBody_.pos, { -40.0f,-30.0f,0.0f });

	// ���[�J����]
	localRotAddLWheel_ = { 0.0f,0.0f,0.0f };

	// ���f������̊�{���
	transformLWheel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_WHEEL));

	// �傫��
	scale = 0.3f;
	transformLWheel_.scl = { scale, scale, scale };

	// ���f������̊�{���X�V
	transformLWheel_.Update();

	// �C�g----------------------------------------------------------

	transformBarrel_.pos = VAdd(transformBody_.pos, { 0.0f,-10.0f,0.0f });

	// ���[�J����]
	localRotAddBarrel_ = { 0.0f,0.0f,0.0f };

	// ���f������̊�{���
	transformBarrel_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::TANK_BARREL));

	// �傫��
	scale = 0.3f;
	transformBarrel_.scl = { scale, scale, scale };

	// ���f������̊�{���X�V
	transformBarrel_.Update();

	//// ���ԗ�
	//MV1SetRotationXYZ(transformLWheel_.modelId, transformLWheel_.rot);
	//MV1SetScale(transformLWheel_.modelId, { 0.3f,0.3f,0.3f });
	//MV1SetPosition(transformLWheel_.modelId, transformLWheel_.pos);

	//// �C�g
	//MV1SetRotationXYZ(transformBarrel_.modelId, transformBarrel_.rot);
	//MV1SetScale(transformBarrel_.modelId, { 0.3f,0.3f,0.3f });
	//MV1SetPosition(transformBarrel_.modelId, transformBarrel_.pos);
	//Update();
	
}

void Tank::Update(void)
{

	// �d�͂ɂ��ړ���
	//CalcGravityPow();

	// �Փ˔���
	Collision();

	// �ړ�
	ProcessMove();

	// �e�̔���
	ProcessShot();

	//SetGoalRotate(localRotAddBody_.y);
	//Rotate();
	transformBody_.quaRot = grvMng_.GetTransform().quaRot;
	transformBody_.quaRot = transformBody_.quaRot.Mult(Quaternion::Euler(0.0f,localRotAddBody_.y,0.0f));
	transform_.quaRot = grvMng_.GetTransform().quaRot;
	transform_.quaRot = transform_.quaRot.Mult(Quaternion::Euler(0.0f, localRotAddBody_.y, 0.0f));
	// �e(���)�Ƃ̑��΍��W���A�e(���)�̉�]���ɍ��킹�ĉ�]
	//transformRWheel_.quaRot = transformRWheel_.quaRot.Mult(transformBody_.quaRot);
	//transformRWheel_.quaRot = transformBody_.quaRot;
	//transformRWheel_.quaRot = transformRWheel_.quaRot.Mult(Quaternion::Euler(localRotAddRWheel_));
	//VECTOR localRotPos = transformBody_.quaRot.PosAxis({ 40.0f,-30.0f,0.0f });
	//transformRWheel_.pos = VAdd(transformBody_.pos, localRotPos);

	//// �e(���)�Ƃ̑��΍��W���A�e(���)�̉�]���ɍ��킹�ĉ�]
	//localRotPos = transformBody_.quaRot.PosAxis({ -40.0f,-30.0f,0.0f });
	//transformLWheel_.pos = VAdd(transformBody_.pos, localRotPos);
	//transformLWheel_.quaRot = transformBody_.quaRot;

	// �e(���)�Ƃ̑��΍��W���A�e(���)�̉�]���ɍ��킹�ĉ�]
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

	// �Փ�(�d��)
	CollisionGravity();

}

void Tank::CollisionGravity(void)
{

	// �d�͕���
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// �d�͕����̔���
	VECTOR dirUpGravity = grvMng_.GetDirUpGravity();

	// �d�͂̋���
	float gravityPow = grvMng_.GetPower();

	// ���f���Ƃ̏Փ˔�����s�����߂̐���(�n�_�A�I�_)���쐬
	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(transformBody_.pos, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(transformBody_.pos, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{
		// �n�ʂƂ̏Փ�
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);
		//if (hit.HitFlag > 0)
		if (hit.HitFlag > 0)
		{

			// �X�Όv�Z�p�ɏՓ˔����ۑ����Ă���
			hitNormal_ = hit.Normal;
			hitPos_ = hit.HitPosition;

			// �Փ˒n�_����A������Ɉړ�
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

void Tank::SyncParent(Transform& transform, VECTOR addAxis, VECTOR localPos)
{

	// �e(���)�̉�]�����擾
	Quaternion parentRot = transformBody_.quaRot;

#pragma region �⑫
	// �C��̃��[�J����]
	// Unity Z��X��Y = Y * X * Z
	//axis = Quaternion::AngleAxis(localRotAdd_.y, AsoUtility::AXIS_Y);
	//localRot = localRot.Mult(axis);

	//axis = Quaternion::AngleAxis(localRotAdd_.x, AsoUtility::AXIS_X);
	//localRot = localRot.Mult(axis);

	//axis = Quaternion::AngleAxis(localRotAdd_.z, AsoUtility::AXIS_Z);
	//localRot = localRot.Mult(axis);

#pragma endregion

	// �e(���)��]����̑��Ή�]
	Quaternion rot = Quaternion::Identity();
	//rot = rot.Mult(Quaternion::Euler({0.0f,0.0f,0.0f}));
	rot = rot.Mult(parentRot);

	// �ғ����̉�]��������
	rot = rot.Mult(Quaternion::Euler(addAxis));

	// �e(���)�̉�]�Ƒ��Ή�]������
	//transform.quaRot = parentRot.Mult(rot);
	transform.quaRot = rot;

	// �e(���)�Ƃ̑��΍��W���A�e(���)�̉�]���ɍ��킹�ĉ�]
	VECTOR localRotPos = Quaternion::PosAxis(parentRot, localPos);

	// ���΍��W�����[���h���W�ɒ���
	transform.pos = VAdd(transformBody_.pos, VScale(localRotPos, 1.0f));

	//// �d�͕����ɉ����ĉ�]������
	//transform.quaRot = grvMng_.GetTransform().quaRot;

	//transform.quaRot = transform.quaRot.Mult(transformBody_.quaRot);

	// ���f������̊�{���X�V
	transform.Update();

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

	// �y��̉�]
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

	// �J���������ɑO�i������
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
	// �Փ�
	DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0x000000);
}
