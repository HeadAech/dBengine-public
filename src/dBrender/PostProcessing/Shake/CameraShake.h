#ifndef CAMERA_SHAKE_H
#define CAMERA_SHAKE_H

class CameraShake
{
	public:
		
		CameraShake() = default;
		CameraShake(const CameraShake&) = delete;
		CameraShake& operator=(const CameraShake&) = delete;
		CameraShake(CameraShake&&) = default;

		static CameraShake &GetInstance() 
		{
            static CameraShake instance;
            return instance;
        }

		void Update(float deltaTime)
		{
			if (m_Elapsed < m_Duration)
			{
				m_Elapsed += deltaTime;
				m_Decay = 1.0f - (m_Elapsed / m_Duration);

				m_CurrentShake = m_Amount * m_Decay;
			}
			else
			{
				m_CurrentShake = 0.0f;
				Active = false;
			}
		}

		void TriggerShake(float intensity, float duration)
		{
            m_Amount = intensity;
            m_Duration = duration;
            m_Elapsed = 0.0f;
            m_CurrentShake = m_Amount;
			Active = true;
		}

		float GetCurrentShake() const { return m_CurrentShake; }
		float GetTime() const { return m_Time; }

		void SetTime(float time) { m_Time = time; }

		bool Active = false;
	private:

		float m_Elapsed = 0.0f;
        float m_Duration = 0.0f;
		float m_Decay = 1.0f;
		float m_Amount = 0.02f;

		float m_CurrentShake = 0.0f;
        float m_Time = 0.0f;
};

#endif // !CAMERA_SHAKE_H
