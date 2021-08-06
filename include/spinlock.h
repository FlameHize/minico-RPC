#pragma once
#include <atomic>
#include "utils.h"

namespace minico {

	//���std::atomic_int����Ķ�Ԫ�ź���ʹ�ã�Ϊ1��ʾ��Դ����ʹ�ã�Ϊ0��ʾ��Դ����ʹ�� �������������ʵ��
	class Spinlock
	{
	public:
		Spinlock()
			: sem_(1)
		{ }

		~Spinlock() { unlock(); }

		DISALLOW_COPY_MOVE_AND_ASSIGN(Spinlock);

		void lock()
		{
			int exp = 1;
			//ԭ�Ӷ�������װ��ֵ�����expected������������ͬ,����ԭ��ֵ���޸Ĳ�����true,���򷵻�false,�������޸�
			//bool compare_exchange_strong (T& expected, T val)
			//�Ƚϲ���������װ��ֵ(strong)����� expected��ָ����ֵ�Ƿ����
			//���,����val�滻ԭ�Ӷ���ľ�ֵ
			//�����,����ԭ�Ӷ���ľ�ֵ�滻expected 
			//��˵��øú���֮���������ԭ�Ӷ����װ��ֵ����� expected ��ָ����ֵ����ȣ�expected �е����ݾ���ԭ�Ӷ���ľ�ֵ
			while (!sem_.compare_exchange_strong(exp, 0))
			{
				exp = 1;
			}
		}

		void unlock()
		{
			sem_.store(1);	//std::atomic::store ����������Ϊ T �Ĳ��� val ���Ƹ�ԭ�Ӷ�������װ��ֵ
		}

	private:
		std::atomic_int sem_;

	};

}