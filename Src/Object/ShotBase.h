#pragma once
#include <DxLib.h>
#include "../Object/Common/Transform.h"

class ShotBase
{

public:

	// �����G�t�F�N�g�̑傫��
	static constexpr float BLAST_SCALE = 10.0f;

	// ���
	enum class STATE
	{
		NONE,
		SHOT,
		BLAST,
		END,
	};

	// �R���X�g���N�^�EInit�ECreate�̋�ʂ���₱�����̂ŁA
	// Init����

	ShotBase(void);

	~ShotBase(void);

	/// <summary>
	/// �e�̐���
	/// </summary>
	/// <param name="birthPos">�������W</param>
	/// <param name="dir">�i�s����</param>
	void Create(VECTOR birthPos, VECTOR dir);

	void Update(void);
	void Draw(void);
	void Release(void);

	// ���W�̎擾
	VECTOR GetPos(void) const;

	// ���W�̐ݒ�
	void SetPos(VECTOR pos);

	// �e�̑��x
	float GetSpeed(void) const;

	// ��������
	float GetTimeAlive(void) const;

	// ���̓����蔻��p�̔��a���擾
	float GetCollisionRadius(void) const;

	// �e�𔚔�������
	void Blast(void);

	ShotBase::STATE GetState(void);

protected:

	// ���f������̊�{���
	Transform transform_;

	// ���
	STATE state_;

	// �i�s����
	VECTOR dir_;

	// �e�̑��x
	float speed_;

	// ��������
	float timeAlive_;
	float stepAlive_;

	float gravityPow_;

	// �����蔻��̔��a
	float collisionRadius_;

	// �����G�t�F�N�g
	int effectBlastResId_;
	int effectBlastPlayId_;

	// �p�����[�^�ݒ�
	void SetParam(void);

	// ��ԕʍX�V����
	void UpdateShot(void);
	void UpdateBlast(void);

	// ��ԑJ��
	void ChangeState(STATE state);

	// �����`�F�b�N
	void CheckAlive(void);

	// �ړ�����
	void Move(void);

	// �����G�t�F�N�g���Đ�
	void PlayBlastEffect(void);

};
