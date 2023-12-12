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

	movedPos_ = { 0.0f,0.0f,0.0f };

	movePow_ = {0.0f,0.0f,0.0f};

	jumpPow_ = { 0.0f,0.0f,0.0f };

	// �J�v�Z���R���C�_
	capsule_ = std::make_unique<Capsule>(transformBody_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, -50.0f, 0.0f });
	capsule_->SetRadius(20.0f);

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



	// �ړ�
	ProcessMove();

	// �e�̔���
	ProcessShot();

	CalcSlope();

	// �d�͂ɂ��ړ���
	CalcGravityPow();

	// �Փ˔���
	Collision();

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

	// ���ݍ��W���N�_�Ɉړ�����W�����߂�
	//movedPos_ = VAdd(transformBody_.pos, movePow_);
	transformBody_.pos = VAdd(transformBody_.pos, movePow_);

	CollisionCapsule();

	// �Փ�(�d��)
	CollisionGravity();

	// �ړ�
	//transformBody_.pos = movedPos_;

}

void Tank::CollisionGravity(void)
{


	// �W�����v�ʂ����Z
	transformBody_.pos = VAdd(transformBody_.pos, jumpPow_);

	// �d�͕���
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// �d�͕����̔���
	VECTOR dirUpGravity = grvMng_.GetDirUpGravity();

	// �d�͂̋���
	float gravityPow = grvMng_.GetPower();

	// ���f���Ƃ̏Փ˔�����s�����߂̐���(�n�_�A�I�_)���쐬
	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(VAdd(transformBody_.pos, {0.0f,100.0f,0.0f}), VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(VAdd(transformBody_.pos, { 0.0f,-100.0f,0.0f }), VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{
		// �n�ʂƂ̏Փ�
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);
		//if (hit.HitFlag > 0)
		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{

			// �X�Όv�Z�p�ɏՓ˔����ۑ����Ă���
			hitNormal_ = hit.Normal;
			hitPos_ = hit.HitPosition;

			// �Փ˒n�_����A������Ɉړ�
			transformBody_.pos = VAdd(hit.HitPosition, VScale(dirUpGravity, 70.0f));

			// �W�����v���Z�b�g
			jumpPow_ = AsoUtility::VECTOR_ZERO;
		}
	}

}

void Tank::CollisionCapsule(void)
{

	// �J�v�Z�����ړ�������
	Transform trans = Transform(transform_);
	trans.pos = transformBody_.pos;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);

	// �J�v�Z���Ƃ̏Փ˔���
	for (const auto c : colliders_)
	{

		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());

		// �Փ˂��������̃|���S���ƏՓˉ������܂ŁA
		// �v���C���[�̈ʒu���ړ�������
		for (int i = 0; i < hits.HitNum; i++)
		{

			auto hit = hits.Dim[i];

			// �n�ʂƈقȂ�A�Փˉ���ʒu���s���Ȃ��߁A���x���ړ�������
			// ���̎��A�ړ�����������́A�ړ��O���W�Ɍ����������ł�������A
			// �Փ˂����|���S���̖@�������������肷��
			for (int tryCnt = 0; tryCnt < 10; tryCnt++)
			{

				// �ēx�A���f���S�̂ƏՓˌ��o����ɂ́A���������߂���̂ŁA
				// �ŏ��̏Փ˔���Ō��o�����Փ˃|���S��1���ƏՓ˔�������
				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{

					// �@���̕����ɂ�����Ƃ����ړ�������
					transformBody_.pos = VAdd(transformBody_.pos, VScale(hit.Normal, 1.0f));
					// �J�v�Z�����ꏏ�Ɉړ�������
					trans.pos = transformBody_.pos;
					trans.Update();
					continue;

				}
				break;
			}
		}

		// ���o�����n�ʃ|���S�����̌�n��
		MV1CollResultPolyDimTerminate(hits);

	}
}

void Tank::CalcGravityPow(void)
{

	// �d�͕���
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// �d�͂̋���
	float gravityPow = grvMng_.GetPower();

	// �d��
	// �d�͂����
	VECTOR gravity = VScale(dirGravity, gravityPow);
	// �����o�ϐ� jumpPow_ �ɏd�͌v�Z���s��(�����x)
	jumpPow_ = VAdd(jumpPow_, gravity);

}

void Tank::CalcSlope(void)
{

	VECTOR gravityUp = grvMng_.GetDirUpGravity();

	// �d�͂̔��Ε�������n�ʂ̖@�������Ɍ�������]�ʂ��擾
	Quaternion up2GNorQua = Quaternion::FromToRotation(gravityUp, hitNormal_);

	// �擾������]�̎��Ɗp�x���擾����
	float angle = 0.0f;
	float* anglePtr = &angle;
	VECTOR axis;
	up2GNorQua.ToAngleAxis(anglePtr, &axis);

	// 90�x�����āA�X�΃x�N�g���ւ̉�]���擾����
	Quaternion slopeQ = Quaternion::AngleAxis(
		angle + AsoUtility::Deg2RadD(90.0), axis);

	// �n�ʂ̌X�ΐ�(���F)
	slopeDir_ = slopeQ.PosAxis(gravityUp);

	// �X�΂̊p�x
	slopeAngleDeg_ = static_cast<float>(
		AsoUtility::AngleDeg(gravityUp, slopeDir_));

	//// �X�΂ɂ��ړ�
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

void Tank::DrawDebug(void)
{
	// �Փ�
	DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0x000000);

	// �J�v�Z���R���C�_
	capsule_->Draw();

}

void Tank::CreateShot(void)
{

	// �e�̐����t���O
	bool isCreate = false;

	for (auto v : shots_)
	{
		if (v->GetState() == ShotBase::STATE::END)
		{
			// �ȑO�ɐ��������C���X�^���X���g����
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
		// ���@�̑O������
		auto dir = transformBarrel_.GetForward();
		// �V�����C���X�^���X�𐶐�
		ShotBase* newShot = new ShotBase();
		newShot->Create({ transformBarrel_.pos.x,
			transformBarrel_.pos.y,
			transformBarrel_.pos.z },
			transformBarrel_.GetForward());

		// �e�̊Ǘ��z��ɒǉ�
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