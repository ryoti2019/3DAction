#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Common/Quaternion.h"
#include "../Object/Common/Transform.h"
#include "../Object/Planet.h"
#include "../Object/Player.h"
#include "GravityManager.h"

GravityManager* GravityManager::instance_ = nullptr;

GravityManager::GravityManager(void)
{
	player_ = nullptr;
	preDirGravity_ = AsoUtility::DIR_D;
	dirGravity_ = AsoUtility::DIR_D;
	dirUpGravity_ = AsoUtility::DIR_U;
	lastHitNormal_ = AsoUtility::DIR_U;
	lastHitPos_ = AsoUtility::VECTOR_ZERO;
	slerpPow_ = 0.001f;
	step_ = 0.0f;

	state_ = STATE::IDLE;
}

void GravityManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new GravityManager();
	}
	instance_->Init();
}

GravityManager& GravityManager::GetInstance(void)
{
	return *instance_;
}

void GravityManager::Init(void)
{

	dirGravity_ = AsoUtility::DIR_D;
	preDirGravity_ = AsoUtility::VECTOR_ZERO;
	step_ = -1.0f;
	slerpPow_ = DEFAULT_POW_SLERP;

	transform_.quaRot = Quaternion();
	transform_.Update();

}

void GravityManager::Update(void)
{

 	switch (state_)
	{
	case GravityManager::STATE::IDLE:
		UpdateIdle();
		break;
	case GravityManager::STATE::STAGE_CHANGE:
		UpdateChangeStage();
		break;
	}

	// �d�͕�����A�������̌v�Z
	Calculate();

}

void GravityManager::UpdateIdle(void)
{
}

void GravityManager::UpdateChangeStage(void)
{
}

void GravityManager::Destroy(void)
{
	delete instance_;
}

void GravityManager::Calculate(void)
{

	// �d�͕���
	dirGravity_ = CalcDirGravity();

	// �d�͂̔��Ε���(�W�����v����)
	dirUpGravity_ = VScale(dirGravity_, -1.0f);

	// ���݂̏����(�܂�A�d�͂̔��Ε���)
	VECTOR up = transform_.GetUp();

	// �Q�̃x�N�g���Ԃ̉�]��(��)�����߂�
	Quaternion rot = Quaternion::FromToRotation(up, dirUpGravity_);

	// ���߂���]�ʂŁA���݂̏d�͐������]������(�������܂�)
	//transform_.quaRot = rot.Mult(transform_.quaRot);

	// �ړI�̊p�x
	goalRot_ = rot;

	// ���X�ɉ�]
	transform_.quaRot = Quaternion::Slerp(transform_.quaRot, goalRot_, static_cast<double>(slerpPow_));

}

std::weak_ptr<Planet> GravityManager::GetActivePlanet(void) const
{
	return activePlanet_;
}

void GravityManager::ChangeActivePlanet(std::weak_ptr<Planet> planet)
{

	// �V�����f�����Z�b�g
	activePlanet_ = planet;

}

void GravityManager::SetPlayer(std::shared_ptr<Player> player)
{
	player_ = player;
}

const Transform& GravityManager::GetTransform(void) const
{
	return transform_;
}

VECTOR GravityManager::GetDirGravity(void) const
{
	return dirGravity_;
}

VECTOR GravityManager::GetDirUpGravity(void) const
{
	return dirUpGravity_;
}

float GravityManager::GetPower(void) const
{
	float ret = 0.0f;
	if (activePlanet_.lock() != nullptr)
	{
		ret = activePlanet_.lock()->GetGravityPow();
	}
	return ret;
}

Quaternion GravityManager::GetPreQuaRot(void) const
{
	return preDirGravity_;
}

void GravityManager::SetLastHitNormal(const VECTOR& normal)
{
	lastHitNormal_ = normal;
}

void GravityManager::SetLastHitPos(const VECTOR& pos)
{
	lastHitPos_ = pos;
}

VECTOR GravityManager::GetLastHitPos(void) const
{
	return lastHitPos_;
}

void GravityManager::ChangeState(STATE state)
{

	state_ = state;
	switch (state_)
	{
	case GravityManager::STATE::IDLE:
		break;
	case GravityManager::STATE::STAGE_CHANGE:
		break;
	}

}

VECTOR GravityManager::CalcDirGravity(void) const
{

	VECTOR ret = AsoUtility::DIR_D;

	if (activePlanet_.lock() == nullptr || player_ == nullptr)
	{
		return ret;
	}

	// �X�e�[�W�^�C�v�ɂ���āA�d�͕����̌v�Z��ς���
	Planet::TYPE stageType = activePlanet_.lock()->GetType();
	switch (stageType)
	{
	case Planet::TYPE::GROUND:
		ret = AsoUtility::DIR_D;
		break;
	case Planet::TYPE::SPHERE:
	{

		// �v���C���[�̍��W
		auto playerPos = player_->GetTransform().pos;
		// �A�N�e�B�u�ɂȂ��Ă���f���̍��W(���̂̒��S���W)
		auto planetPos = activePlanet_.lock()->GetTransform().pos;

		// �d�͕��������߂�
		auto gravityVec = VSub(planetPos, playerPos);
		auto gravityDir = VNorm(gravityVec);
		ret = gravityDir;

	}
	break;
	case Planet::TYPE::TRANS_ROT:
	{
		// �d�͕��������߂�
		ret = activePlanet_.lock()->GetTransform().GetDown();
	}
	break;
	case Planet::TYPE::TRANS_CAMERA_FIXED:
		break;
	case Planet::TYPE::ROAD:
	{
		// �v���C���[�̐��x�N�g��
		auto hitNormal = player_->GetHitNormal();

		// �d�͕��������߂�
		ret = VScale(hitNormal,-1.0f);
		break;
	}

	return ret;

	}
}
