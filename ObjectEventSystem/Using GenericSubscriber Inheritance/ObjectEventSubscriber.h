#pragma once

namespace SB
{	
	class GenericSubscriber
	{
	public:
		virtual ~GenericSubscriber()
		{}

		virtual uint16_t GetTypeIndex() const = 0;
		virtual void TriggerObjectEvent(const void * aEvent) = 0;
	};

	template<typename TEvent>
	class ObjectEventSubscriber : public GenericSubscriber
	{
	public:

		uint16_t GetTypeIndex() const override
		{
			return UniqueIdentifier<GenericSubscriber>::GetID<TEvent>();
		}

		virtual void ReceiveObjectEvent(const TEvent & aEvent) = 0;

		virtual void TriggerObjectEvent(const void * aEvent) override
		{
			ReceiveObjectEvent(*static_cast<const TEvent*>(aEvent));
		}
	};
}
