#include "SafeObject.h"


void ImageUtil::SafeObject::buildOnReleaseAction(const IOnObjectRelease release)
{
	onObjectRelease = release;
}

ImageUtil::SafeObject::SafeObject()
{
	referenceCount++;
}


ImageUtil::SafeObject::~SafeObject()
{
	referenceCount--;
	if (referenceCount == 0)
	{
		if (onObjectRelease != nullptr)
			onObjectRelease();
	}

}

ImageUtil::SafeObject& ImageUtil::SafeObject::operator=(const SafeObject& other)
{
	if(this == &other)
	{
		return *this;
	}

	referenceCount--;
	if (referenceCount == 0)
		if (onObjectRelease != nullptr)
			onObjectRelease();

	onObjectRelease = other.onObjectRelease;
	referenceCount = other.referenceCount;
	referenceCount++;

	return *this;
}




ImageUtil::SafeObject::SafeObject(const SafeObject& other)
{
	referenceCount++;
}

ImageUtil::SafeObject::SafeObject(SafeObject&& other) noexcept
{
}
