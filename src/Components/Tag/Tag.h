#ifndef TAG_H
#define TAG_H

#include "Component/Component.h"
#include <string>
#include <string_view>

class Tag : public Component
{

	public:
		
		Tag(std::string_view name);
		~Tag() = default;

		std::string Name = "None";
};


#endif // !TAG_H
