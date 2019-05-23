#pragma once
namespace ImageUtil {
	
	
	class SafeObject 
	{
	public:
		typedef void (*IOnObjectRelease)();
		typedef unsigned int uint;
	private:
		uint referenceCount = 0;

	protected:
		IOnObjectRelease onObjectRelease = nullptr;
		void buildOnReleaseAction(IOnObjectRelease release);

	public:
		SafeObject();
		SafeObject(const SafeObject& other);
		SafeObject(SafeObject&& other) noexcept;
		virtual ~SafeObject();

		SafeObject& operator=(const SafeObject& other);
		SafeObject& operator=(SafeObject&& ohter) = delete;

	};

}