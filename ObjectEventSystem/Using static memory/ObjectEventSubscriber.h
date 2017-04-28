#pragma once

namespace SB
{	
	class GenericSubscriber;

	template<typename TEvent>
	class ObjectEventSubscriber
	{
	public:
		ObjectEventSubscriber()
		{
			myIndex = 0;
			AddSubscriber();
		}
		virtual ~ObjectEventSubscriber()
		{
			RemoveSubscriber();
		}
		
		uint16_t GetTypeIndex() const
		{
			return UniqueIdentifier<GenericSubscriber>::GetID<TEvent>();
		}

		uint16_t GetInstanceIndex() const
		{
			return myIndex;
		}

		virtual void ReceiveObjectEvent(const TEvent & aEvent) = 0;

		static void TriggerObjectEvent(const uint16_t aIndex, const TEvent & aEvent)
		{
			ourSubscribers[static_cast<uint16_t>(aIndex)]->ReceiveObjectEvent(aEvent);
		}

	private:
		void AddSubscriber()
		{
			if (ourFreeIndexes.Size() <= 0)
			{
				uint16_t growSize = ourSubscribers.Size();
				const uint16_t endPoint = growSize;

				if (growSize < 1)
				{
					growSize = 2;
				}

				ourSubscribers.Resize(growSize * 2);

				for (uint16_t iFreeIndex = ourSubscribers.Size() -1; iFreeIndex >= endPoint; --iFreeIndex)
				{
					ourFreeIndexes.Push(iFreeIndex);
					
					if (iFreeIndex == 0)
					{
						break;
					}
				}
			}

			myIndex = ourFreeIndexes.Pop();
			ourSubscribers[myIndex] = this;
		}

		void RemoveSubscriber()
		{
			ourFreeIndexes.Push(myIndex);
		}

		static Stack<uint16_t> ourFreeIndexes;
		static GrowingArray<ObjectEventSubscriber<TEvent> *> ourSubscribers;
		uint16_t myIndex;
	};

	template<typename TEvent>
	GrowingArray<ObjectEventSubscriber<TEvent> *> ObjectEventSubscriber<TEvent>::ourSubscribers;

	template<typename TEvent>
	Stack<uint16_t> ObjectEventSubscriber<TEvent>::ourFreeIndexes;
}
