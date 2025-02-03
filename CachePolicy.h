#pragma once
#pragma once

namespace FgCache
{
    template <typename Key, typename Value>
    class FgCachePolicy
    {
    public:
        // ��ӻ���ӿ�
        virtual void put(Key key, Value value) = 0;

        // key �Ǵ�����������ʵ���ֵ�Դ�����������ʽ���أ����ʳɹ����� true
        virtual bool get(Key key, Value& value) = 0;

        // ������������ҵ� key ��ֱ�ӷ��� value
        virtual Value get(Key key) = 0;

        virtual ~FgCachePolicy() {};
    };
}