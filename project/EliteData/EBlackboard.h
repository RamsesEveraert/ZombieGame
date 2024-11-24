#pragma once

#include <unordered_map>
#include <string>
#include <memory>

namespace Elite
{
    class IBlackBoardField
    {
    public:
        IBlackBoardField() = default;
        virtual ~IBlackBoardField() = default;
    };

    template<typename T>
    class BlackboardField : public IBlackBoardField
    {
    public:
        explicit BlackboardField(T data) : m_Data(data) {}
        T GetData() { return m_Data; }
        void SetData(T data) { m_Data = data; }

    private:
        T m_Data;
    };

    class Blackboard final
    {
    public:
        Blackboard() = default;
        ~Blackboard() { };

        Blackboard(const Blackboard& other) = delete;
        Blackboard& operator=(const Blackboard& other) = delete;
        Blackboard(Blackboard&& other) = delete;
        Blackboard& operator=(Blackboard&& other) = delete;

        template<typename T>
        bool AddData(const std::string& name, T data)
        {
            auto it = m_BlackboardData.find(name);
            if (it == m_BlackboardData.end())
            {
                m_BlackboardData[name] = std::make_unique<BlackboardField<T>>(data);
                return true;
            }
            printf("WARNING: Data '%s' of type '%s' already in Blackboard \n", name.c_str(), typeid(T).name());
            return false;
        }

        template<typename T>
        bool ChangeData(const std::string& name, T data)
        {
            auto it = m_BlackboardData.find(name);
            if (it != m_BlackboardData.end())
            {
                BlackboardField<T>* p = dynamic_cast<BlackboardField<T>*>(it->second.get());
                if (p)
                {
                    p->SetData(data);
                    return true;
                }
            }
            printf("WARNING: Data '%s' of type '%s' not found in Blackboard \n", name.c_str(), typeid(T).name());
            return false;
        }

        template<typename T>
        bool GetData(const std::string& name, T& data)
        {
            auto it = m_BlackboardData.find(name);
            if (it != m_BlackboardData.end())
            {
                BlackboardField<T>* p = dynamic_cast<BlackboardField<T>*>(it->second.get());
                if (p != nullptr)
                {
                    data = p->GetData();
                    return true;
                }
            }
            printf("WARNING: Data '%s' of type '%s' not found in Blackboard \n", name.c_str(), typeid(T).name());
            return false;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<IBlackBoardField>> m_BlackboardData;
    };
}
