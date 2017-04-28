#pragma once
#include "ObjectEventSystem\ObjectEventSubscriber.h"

class Object
{
public:
	

	Object()
	{}
	~Object()
	{}

	/*
		other code
	*/


	template <typename TEvent>
	void TriggerObjectEvent(const TEvent & aEvent) const;
	{
		if (IsRemoved())
		{
			return;
		}

		const uint16_t EventID = UniqueIdentifier<GenericSubscriber>::GetID<TEvent>();

		if (EventID < myEventSubscribers.Size())
		{
			for (uint16_t iSubscriber = 0; iSubscriber < myEventSubscribers[EventID].Size(); ++iSubscriber)
			{
				myEventSubscribers[EventID][iSubscriber]->TriggerObjectEvent(&aEvent);
			}
		}
	}

	template<typename TEventType>
	void AddObjectEventSubscriber(ObjectEventSubscriber<TEventType> & aEventSubscriber)
	{
		const uint16_t Index = aEventSubscriber.GetTypeIndex();

		if (myEventSubscribers.Size() <= Index)
		{
			myEventSubscribers.Resize(Index + 1);
		}

		myEventSubscribers[Index].Add(&aEventSubscriber);
	}

	
private:
	GrowingArray<GrowingArray<GenericSubscriber*>>  myEventSubscribers;
	
};
	
