/*
 * Copyright (c) 2019, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>
#include <modm/driver/pwm/ws2812b.hpp>
// #include <modm/driver/pwm/sk6812w.hpp>
#include <modm/ui/led/tables.hpp>
#include <modm/processing/timer.hpp>

using namespace Board;

using Output = Board::D11;
modm::Ws2812b<SpiMaster2, Output, 8*32> leds;
// modm::Sk6812w<SpiMaster2, Output, 144> leds;
modm::ShortPeriodicTimer tmr{50ms};

int
main()
{
	Board::initialize();
	LedD13::setOutput();
	leds.initialize<Board::SystemClock>();

	// constexpr uint8_t max = 254;
	// uint8_t r=0, g=max/3, b=max/3*2;

	while (true)
	{
		// for (size_t ii=0; ii < leds.size; ii++)
		// {
		// 	leds.setColor(ii,
		// 				  {0xff,
		// 				   0xff,
		// 				   0xff});
		// 	leds.setBrightness(ii,0x00);
		// }
		// leds.write();

		// while(not tmr.execute()) ;

		for (size_t i=0; i<leds.size; i++)
		{

			for (size_t ii=0; ii < leds.size; ii++)
			{
				leds.setColor(ii,
							{0x00,
							0x00,
							0x00});
				// leds.setBrightness(ii,0xff);
			}
			leds.setColor(i, {0xff, 0xff, 0xff});
			leds.write();

			while(not tmr.execute()) ;
		}

		// for (size_t ii=0; ii < leds.size; ii++)
		// {
		// 	leds.setColor(ii,
		// 				  {0x00,
		// 				   0x00,
		// 				   0x00});
		// 	// leds.setBrightness(ii,0xff);
		// }
		// leds.write();

		// while(not tmr.execute()) ;

		// 		for (size_t ii=0; ii < leds.size; ii++)
		// {
		// 	leds.setColor(ii,
		// 				  {0x00,
		// 				   0x00,
		// 				   0x00});
		// 	leds.setBrightness(ii,0x00);
		// }
		// leds.write();

		// while(not tmr.execute()) ;
		LedD13::toggle();
	}

	return 0;
}
