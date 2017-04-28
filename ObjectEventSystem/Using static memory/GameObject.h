#pragma once

class GameObject
{
public:
	GameObject(Scene & aScene, const ID aID, const std::string & aIdentifier, std::atomic<int> & aReferenceCounter);

	GameObject(const GameObject &aOtherGameObject);
	~GameObject();

	/*
		OTHER CODE
	*/

	template <typename TEvent>
	void TriggerObjectEvent(const TEvent & aEvent) const
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
				const uint16_t tempSubscriberIndex = myEventSubscribers[EventID][iSubscriber];
				ObjectEventSubscriber<TEvent>::TriggerObjectEvent(tempSubscriberIndex, aEvent);
			}
		}
	}

	template<typename TEventType>
	void AddObjectEventSubscriber(const ObjectEventSubscriber<TEventType> & aEventSubscriber)
	{
		const uint16_t Index = aEventSubscriber.GetTypeIndex();
		const uint16_t Subscriber = aEventSubscriber.GetInstanceIndex();

		if (myEventSubscribers.Size() <= Index)
		{
			myEventSubscribers.Resize(Index + 1);
		}

		myEventSubscribers[Index].Add(Subscriber);
	}

	
private:
	GrowingArray<GrowingArray<uint16_t>>  myEventSubscribers;
};



