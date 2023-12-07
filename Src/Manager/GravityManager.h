#pragma once
#include <memory>
#include "../Object/Common/Transform.h"
#include "../Common/Quaternion.h"
class SceneManager;
class Planet;
class Player;
class Tank;

class GravityManager
{

public:

	// �d�͕��������S�ɕς��܂ł̎���
	static constexpr float TIME_CHANGE_DIR = 1.0f;

	// ��]��
	static constexpr float DEFAULT_POW_SLERP = 0.08f;

	// ���
	enum class STATE
	{
		IDLE,
		STAGE_CHANGE
	};

	// �C���X�^���X�̐���
	static void CreateInstance(void);

	// �C���X�^���X�̎擾
	static GravityManager& GetInstance(void);

	void Init(void);

	void Update(void);
	void UpdateIdle(void);
	void UpdateChangeStage(void);

	void Destroy(void);

	// �d�͕�����A�������̌v�Z
	void Calculate(void);

	// ���݃v���C���Ă���f��
	std::weak_ptr<Planet> GetActivePlanet(void) const;

	// ���݃v���C���Ă���f����ύX����
	void ChangeActivePlanet(std::weak_ptr<Planet> planet);

	// ����v���C���[���Z�b�g
	void SetPlayer(std::shared_ptr<Player> player);

	// ����^���N���Z�b�g
	void SetTank(std::shared_ptr<Tank> tank);

	const Transform& GetTransform(void) const;
	
	// �d�͕������擾
	VECTOR GetDirGravity(void) const;

	// �d�͕����̔��Ε������擾
	VECTOR GetDirUpGravity(void) const;

	// �d�͂̋������擾
	float GetPower(void) const;

	// �f���؂�ւ��O�̉�]���
	Quaternion GetPreQuaRot(void) const;

	// �v���C���[�Ƒ����Փ˂����@��
	void SetLastHitNormal(const VECTOR& normal);

	// �v���C���[�Ƒ����Փ˂����ʒu
	void SetLastHitPos(const VECTOR& pos);
	VECTOR GetLastHitPos(void) const;

protected:

	// �ÓI�C���X�^���X
	static GravityManager* instance_;

	// �v���C���[
	std::shared_ptr<Player> player_;

	// �^���N
	std::shared_ptr<Tank> tank_;

	// ���݃v���C���Ă���f��
	std::weak_ptr<Planet> activePlanet_;

	Transform transform_;

	// ���
	STATE state_;

	// �d�͕���
	VECTOR dirGravity_;
	VECTOR preDirGravity_;

	// �d�͕����̔��Ε���
	VECTOR dirUpGravity_;

	// �v���C���[�Ƒ����Փ˂����@��
	VECTOR lastHitNormal_;

	// �v���C���[�Ƒ����Փ˂����ʒu
	VECTOR lastHitPos_;

	// �X�V�X�e�b�v
	float step_;

	// ��]��
	float slerpPow_;

	// �ړI�̊p�x
	Quaternion goalRot_;

	// �f�t�H���g�R���X�g���N�^��private�ɂ��āA
	// �O�����琶���ł��Ȃ��l�ɂ���
	GravityManager(void);
	GravityManager(const GravityManager& manager) = default;
	~GravityManager(void) = default;

	// ��ԑJ��
	void ChangeState(STATE state);

	// �d�͕������v�Z
	VECTOR CalcDirGravity(void) const;

};
