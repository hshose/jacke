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
#include <modm/ui/color/rgb.hpp>
#include <modm/ui/color/rgb565.hpp>
#include <modm/processing.hpp>

#include <array>
#include <utility>
#include <algorithm>

#include "led_jacket.hpp"

using namespace Board;

using Output = Board::D11;
// modm::Ws2812b<SpiMaster2, Output, 8*32> leds;
// modm::Sk6812w<SpiMaster2, Output, 144> leds;

using DmaRx = Dma1::Channel3;
using DmaTx = Dma1::Channel4;
using SpiLed = SpiMaster2_Dma<DmaRx, DmaTx>;
jacket::JacketBack<SpiLed, Output> jacket_back;

// jacket::BeatDetection<> beat_detection;

modm::ShortPeriodicTimer tmr{50ms};

class JacketThread : public modm::pt::Protothread
{
public:
	bool
	update()
	{
		PT_BEGIN();
		Dma1::enable();
		jacket_back.initialize<Board::SystemClock>();

		while (true)
		{
			jacket_back.update();
			PT_CALL(jacket_back.leds.write());
			timeout.restart(500ms);
			PT_WAIT_UNTIL(timeout.isExpired());
		}

		PT_END();
	}
private:
	modm::ShortTimeout timeout;
};

JacketThread jacket_thread;

int
main()
{
	Board::initialize();
	LedD13::setOutput();
	// beat_detection.initialize();

	while (true)
	{
		// while(not tmr.execute()) ;
		jacket_thread.update();
		LedD13::toggle();
	}

	return 0;
}
