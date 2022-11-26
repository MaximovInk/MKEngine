#pragma once
#include "mkpch.h"

namespace MKEngine {
	using MouseCode = uint8_t;

	namespace Mouse {


		enum : MouseCode {
			NONE = 0,
			Button0 = 1,
			Button1 = 2,
			Button2 = 3,
			Button3 = 4,
			Button4 = 5,

			LeftButton = Button0,
			MiddleButton = Button1,
			RightButton = Button2,
			X1Button = Button3,
			X2Button = Button4,
		};
	}
}