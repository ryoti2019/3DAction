#pragma once
#include "Common/Transform.h"
#include "ActorBase.h"

class Collider;
class ShotBase;
class Capsule;

class Tank : public ActorBase
{
public:

	// ��]�����܂ł̎���
	static constexpr float TIME_ROT = 1.0f;

	// �e�̔��ˊԊu
	static constexpr float TIME_DELAY_SHOT = 1.0f;

	// �R���X�g���N�^
	Tank(void);

	// �f�X�g���N�^
	~Tank(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

	// �Փ˔���ɗp������R���C�_����
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

private:

	// �e
	std::vector<ShotBase*> shots_;

	// �Փ˔���ɗp������R���C�_
	std::vector<std::weak_ptr<Collider>> colliders_;

	std::unique_ptr<Capsule> capsule_;

	// �Փ˃`�F�b�N
	VECTOR gravHitPosDown_;

	VECTOR gravHitPosUp_;

	// �{�f�B�̊�{���
	Transform transformBody_;

	// �E�ԗւ̊�{���
	Transform transformRWheel_;

	// ���ԗւ̊�{���
	Transform transformLWheel_;

	// �C�g�̊�{���
	Transform transformBarrel_;

	Quaternion playerRotY_;
	Quaternion goalQuaRot_;

	float stepRotTime_;

	// �y��̉�]����~�ϗp
	VECTOR localRotAddBody_;

	// �ԗւ̉�]����~�ϗp
	VECTOR localRotAddRWheel_;
	VECTOR localRotAddLWheel_;

	// �C�g�̉�]����~�ϗp
	VECTOR localRotAddBarrel_;

	// �ړ���̍��W
	VECTOR movedPos_;

	// �W�����v��
	VECTOR jumpPow_;

	// �ړ���
	VECTOR movePow_;

	// �����Փ˂��Ă���n�ʂ̃|���S���ʂ̖@��
	VECTOR hitNormal_;

	// �����Փ˔��肵�Ă���n�ʃ|���S���̈ʒu
	VECTOR hitPos_;

	float delayShot_;

	// �X�΂̕���
	VECTOR slopeDir_;

	// �X�Έړ����J�n����X�Ίp�x(�f�O���[)
	float slopeAngleDeg_;

	// �X�Έړ����̈ړ���
	VECTOR slopePow_;

	// �Փ˔���
	void Collision(void);
	void CollisionGravity(void);

	void CollisionCapsule(void);

	void CalcSlope(void);

	// �ړ��ʂ̌v�Z
	void CalcGravityPow(void);

	// ��]
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// �e(���)�Ƃ̉�]�ƈʒu�̓���
	void SyncParent(Transform& transform, VECTOR addAxis, VECTOR localPos);

	// �ړ�����
	void ProcessMove(void);

	// ���ˑ���
	void ProcessShot(void);

	void DrawDebug(void);

	void CreateShot(void);

};