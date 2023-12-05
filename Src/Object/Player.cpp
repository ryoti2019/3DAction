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

	// �Փ˃`�F�b�N
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	isJump_ = false;
	imgShadow_ = -1;

	reserveStartPos_ = AsoUtility::VECTOR_ZERO;
	stepWarp_ = 0.0f;
	timeWarp_ = 0.0f;
	warpReservePos_ = AsoUtility::VECTOR_ZERO;

	preWarpName_ = Stage::NAME::MAIN_PLANET;

	// ��ԊǗ�
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

	// ���f���̊�{�ݒ�
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	transform_.scl = AsoUtility::VECTOR_ONE;
	transform_.pos = { 0.0f, -30.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();

	// �A�j���[�V�����̐ݒ�
	InitAnimation();

	// �ۉe�摜
	imgShadow_ = resMng_.Load(
		ResourceManager::SRC::PLAYER_SHADOW).handleId_;

	// ����
	effectSmokeResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FOOT_SMOKE).handleId_;

	// ����̃G�t�F�N�g
	effectHandLResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::WARP_ORBIT).handleId_;

	// �E��̃G�t�F�N�g
	effectHandRResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::WARP_ORBIT).handleId_;

	// �J�v�Z���R���C�_
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 30.0f, 0.0f });
	capsule_->SetRadius(20.0f);

	// �������
	ChangeState(STATE::PLAY);

}

void Player::Update(void)
{

	// �X�V�X�e�b�v
	stateUpdate_();

	transform_.Update();

	// �A�j���[�V�����̍X�V
	animationController_->Update();
	
}

void Player::Draw(void)
{

	// ���f���̕`��
	MV1DrawModel(transform_.modelId);

	// �ۉe�`��
	DrawShadow();

	// �f�o�b�O�p�`��
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

	// ���[�v��������
	timeWarp_ = time;

	// ���[�v�����o�ߎ���
	stepWarp_ = time;

	// ���[�v�����������̉�]
	warpQua_ = goalRot;

	// ���[�v�����������̍��W
	warpReservePos_ = goalPos;

	// ���[�v�O�̘f������ێ�
	preWarpName_ = grvMng_.GetActivePlanet().lock()->GetName();

	ChangeState(STATE::WARP_RESERVE);

}

void Player::CollisionCapsule(void)
{

	// �J�v�Z�����ړ�������
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
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
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
					// �J�v�Z�����ꏏ�Ɉړ�������
					trans.pos = movedPos_;
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

	// ��ԕύX
	state_ = state;

	// �e��ԑJ�ڂ̏�������
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

	// ���[�v�����J�n���̃v���C���[���
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

	// �ړ�����
	ProcessMove();

	// �ړ������ɉ�������]
	Rotate();

	// �W�����v����
	ProcessJump();

	CalcSlope();

	// �d�͂ɂ��ړ���
	CalcGravityPow();

	// �Փ˔���
	Collision();

	// �d�͕����ɉ����ĉ�]������
	transform_.quaRot = grvMng_.GetTransform().quaRot;
	transform_.quaRot = transform_.quaRot.Mult(playerRotY_);

	// �����G�t�F�N�g
	EffectFootSmoke();

}

void Player::UpdateWarpReserve(void)
{

	stepWarp_ -= SceneManager::GetInstance().GetDeltaTime();

	if (stepWarp_ < 0.0f)
	{

		// ���[�v����������(2�b�o��)
		transform_.quaRot = warpQua_;
		transform_.pos = warpReservePos_;

		ChangeState(STATE::WARP_MOVE);

	}
	else
	{

		// ���[�v�������ŁA��]�ƍ��W����
		// ���`��Ԃ��s��
		transform_.pos = AsoUtility::Lerp(reserveStartPos_, warpReservePos_, 1.0f - (stepWarp_ / timeWarp_));

		// ���ʕ�Ԃ��s��
		transform_.quaRot = Quaternion::Slerp(reserveStartQua_, warpQua_, 1.0f - (stepWarp_ / timeWarp_));

	}

}

void Player::UpdateWarpMove(void)
{

	// �ړ����������W�{�ړ���
	// �ړ��ʁ@�������~�X�s�[�h

	transform_.pos = VAdd(transform_.pos, VScale(transform_.GetForward(), 20.0f));
	transform_.Update();
	
	SyncHandEffect();

	// ���̘f���ɐ؂�ւ�����烏�[�v��Ԃ���v���C��Ԃ֐؂�ւ���
	Stage::NAME name = grvMng_.GetActivePlanet().lock()->GetName();
	if (name != preWarpName_)
	{
		// �����A�j���[�V����
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

	// �L������{���
	//-------------------------------------------------------
	// �L�������W
	v = transform_.pos;
	DrawFormatString(20, 60, black, "�L�������W �F (%0.2f, %0.2f, %0.2f)",
		v.x, v.y, v.z
	);
	//-------------------------------------------------------

	//// �Փ�
	//DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0x000000);

	//// �J�v�Z���R���C�_
	//capsule_->Draw();

	// �A�N�e�B�u�Șf��
	DrawFormatString(20, 80, black, "�f���@�@�@ �F %d",
		(int)grvMng_.GetActivePlanet().lock()->GetName()
	);

}

void Player::ProcessMove(void)
{

	auto& ins = InputManager::GetInstance();

	// �J�����̉�]���
	Quaternion cameraRot = mainCamera->GetQuaRotOutX();

	// �ړ��ʂ��[��
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;
	// ��]�������p�x
	double rotRad = 0;

	// �J���������ɑO�i������
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

		// �ړ�����
		speed_ = SPEED_MOVE;
		if (ins.IsNew(KEY_INPUT_RSHIFT))
		{
			speed_ = SPEED_RUN;
		}
		moveDir_ = dir;
		movePow_ = VScale(dir, speed_);

		// ��]����
		SetGoalRotate(rotRad);

		if (!isJump_ && IsEndLanding())
		{
			// �A�j���[�V����
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

	// ���ݐݒ肳��Ă����]�Ƃ̊p�x�������
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// �������l
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}

	goalQuaRot_ = axis;

}

void Player::Rotate(void)
{

	stepRotTime_ -= scnMng_.GetDeltaTime();

	// ��]�̋��ʕ��
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

}

void Player::Collision(void)
{

	// ���ݍ��W���N�_�Ɉړ�����W�����߂�
	movedPos_ = VAdd(transform_.pos, movePow_);

	// �Փ�(�J�v�Z��)
	CollisionCapsule();

	// �Փ�(�d��)
	CollisionGravity();

	// �ړ�
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;

}

void Player::CollisionGravity(void)
{

	// �W�����v�ʂ����Z
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// �d�͕���
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// �d�͕����̔���
	VECTOR dirUpGravity = grvMng_.GetDirUpGravity();

	// �d�͂̋���
	float gravityPow = grvMng_.GetPower();

	// ���f���Ƃ̏Փ˔�����s�����߂̐���(�n�_�A�I�_)���쐬
	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
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
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));
			// �W�����v���Z�b�g
			jumpPow_ = AsoUtility::VECTOR_ZERO;
			stepJump_ = 0.0f;
			if (isJump_)
			{
				// ���n���[�V����
				animationController_->Play(
					(int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);
			}
			isJump_ = false;
		}
	}

}

void Player::CalcGravityPow(void)
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

	// ����
	float dot = VDot(dirGravity, jumpPow_);
	if (dot >= 0.0f)
	{
		// �d�͕����Ɣ��Ε���(�}�C�i�X)�łȂ���΁A�W�����v�͂𖳂���
		jumpPow_ = gravity;
	}

}

void Player::CalcSlope(void)
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

	// �X�΂ɂ��ړ�
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

	// �W�����v
	if (isHit && (isJump_ || IsEndLanding()))
	{
		if (!isJump_)
		{
			// ���䖳���W�����v
			//animationController_->Play((int)ANIM_TYPE::JUMP);

			// ���[�v���Ȃ��W�����v
			//animationController_->Play((int)ANIM_TYPE::JUMP,false);

			// �؂���W�����v
			//animationController_->Play((int)ANIM_TYPE::JUMP, false, 13.0f, 24.0f);

			// �������A�j���[�V����
			animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
			animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);

		}
		isJump_ = true;
		// �W�����v�̓��͎�t���Ԃ��w���X
		stepJump_ += scnMng_.GetDeltaTime();
		if (stepJump_ < TIME_JUMP_IN)
		{
			 jumpPow_ = VScale(grvMng_.GetDirUpGravity(), POW_JUMP);
		}
	}

	// �{�^���𗣂�����W�����v�͂ɉ��Z���Ȃ�
	if (!isHit)
	{
		stepJump_ = TIME_JUMP_IN;
	}

}

bool Player::IsEndLanding(void)
{

	bool ret = true;

	// �A�j���[�V�������W�����v�ł͂Ȃ�
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::JUMP)
	{
		return ret;
	}

	// �A�j���[�V�������I�����Ă��邩
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

	// �v���C���[�̉e��`��
	int i, j;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3];
	VECTOR SlideVec;
	int ModelHandle;

	// ���C�e�B���O�𖳌��ɂ���
	SetUseLighting(FALSE);

	// �y�o�b�t�@��L���ɂ���
	SetUseZBuffer3D(TRUE);

	// �e�N�X�`���A�h���X���[�h�� CLAMP �ɂ���( �e�N�X�`���̒[����͒[�̃h�b�g�����X���� )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// �e�𗎂Ƃ����f���̐������J��Ԃ�
	for (auto c : colliders_)
	{

		// ���f���̃n���h��ID
		ModelHandle = c.lock()->modelId_;

		// �v���C���[�̒����ɑ��݂���n�ʂ̃|���S�����擾
		HitResDim = MV1CollCheck_Capsule(ModelHandle, -1, transform_.pos, VAdd(transform_.pos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f)), PLAYER_SHADOW_SIZE);

		// ���_�f�[�^�ŕω��������������Z�b�g
		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
		Vertex[0].su = 0.0f;
		Vertex[0].sv = 0.0f;
		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		// ���̒����ɑ��݂���|���S���̐������J��Ԃ�
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			// �|���S���̍��W�͒n�ʃ|���S���̍��W
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			// ������Ǝ����グ�ďd�Ȃ�Ȃ��悤�ɂ���
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			// �|���S���̕s�����x��ݒ肷��
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[0].dif.a = 128 * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[1].dif.a = 128 * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[2].dif.a = 128 * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT);

			// �t�u�l�͒n�ʃ|���S���ƃv���C���[�̑��΍��W���犄��o��
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;

			// �e�|���S����`��
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		// ���o�����n�ʃ|���S�����̌�n��
		MV1CollResultPolyDimTerminate(HitResDim);
	}
	// ���C�e�B���O��L���ɂ���
	SetUseLighting(TRUE);

	// �y�o�b�t�@�𖳌��ɂ���
	SetUseZBuffer3D(FALSE);

}

void Player::EffectFootSmoke(void)
{

	// �G�t�F�N�g�Đ�
	effectSmokePlayId_ = PlayEffekseer3DEffect(effectSmokeResId_);

	// �傫��
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

	// �G�t�F�N�g�Đ�
	effectHandLPlayId_ = PlayEffekseer3DEffect(effectHandLResId_);
	effectHandRPlayId_ = PlayEffekseer3DEffect(effectHandRResId_);

}

void Player::SyncHandEffect(void)
{

	// ���W
	auto leftPos = MV1GetFramePosition(transform_.modelId, 11);
	auto rightPos = MV1GetFramePosition(transform_.modelId, 35);

	// �傫��
	float SCALE = 10.0f;

	// �肩��o��G�t�F�N�g
	// ����
	SetScalePlayingEffekseer3DEffect(effectHandLPlayId_, SCALE, SCALE, SCALE);
	SetPosPlayingEffekseer3DEffect(effectHandLPlayId_, leftPos.x, leftPos.y, leftPos.z);
	SetRotationPlayingEffekseer3DEffect(effectHandLPlayId_, transform_.rot.x, transform_.rot.y, transform_.rot.z);

	// �E��
	SetScalePlayingEffekseer3DEffect(effectHandRPlayId_, SCALE, SCALE, SCALE);
	SetPosPlayingEffekseer3DEffect(effectHandRPlayId_, rightPos.x, rightPos.y, rightPos.z);
	SetRotationPlayingEffekseer3DEffect(effectHandRPlayId_, transform_.rot.x, transform_.rot.y, transform_.rot.z);

}
