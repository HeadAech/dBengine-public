#include "Tag.h"


Tag::Tag(std::string_view name)
{
	// setting component name
	this->name = "Tag";
	this->icon = ICON_FA_TAG;

	Name = name;
}