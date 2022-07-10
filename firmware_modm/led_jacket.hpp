#include <modm/driver/pwm/ws2812b.hpp>

/*
 *         x  0 1 2 3 ...
 *           ──────────────────────────────────────────►
 *           panel  0                       1      ...
 *           columns                                  ▲
 *  y                                                 │
 *    │panel ┌─────────────────┐      ┌───────────────┼─┐
 *  0 │rows  │        0        │      │               │ │
 *  1 │      │ xxxxxxxx──────┐ │      │ ┌────────────►┘ │
 *  2 │      │               │ │      │ │               │
 *  3 │      │ xxxxx◄────────┘ │      │ └◄────────────┐ │
 * ...│      │                 │      │               │ │
 *    │      │ xx────────────┐ │      │ ┌────────────►┘ │
 *    │ 0    │               │ │      │ │               │
 *    │      │ ┌◄────────────┘ │      │ └◄────────────┐ │
 *    │      │ │               │      │               │ │
 *    │      │ └────────────►┐ │      │ ┌────────────►┘ │
 *    │      │               │ │      │ │               │
 *    │      │ ┌◄────────────┘ │      │ └◄────────── ◄┐ │
 *    │      │ │               │      │               │ │
 *    │      └─┼───────────────┘      └───────────────┼─┘
 *    │        │                                      │
 *    │        │                                      │
 *    │      ┌─┼───────────────┐      ┌───────────────┼─┐
 *    │      │ │               │      │               │ │
 *    │      │ └► ───────────┐ │      │ ┌────────────►┘ │
 *    │      │               │ │      │ │               │
 *    │      │ ┌◄────────────┘ │      │ └◄────────────┐ │
 *    │      │ │               │      │               │ │
 *    │ 1    │ └────────────►┐ │      │ ┌────────────►┘ │
 *    │      │               │ │      │ │               │
 *    │      │ ┌◄────────────┘ │      │ └◄────────────┐ │
 *    │      │ │               │      │               │ │
 *    │      │ └────────────►┐ │      │ ┌────────────►┘ │
 *    │      │               │ │      │ │               │
 *    │      │ ┌◄────────────┘ │      │ └◄────────────┐ │
 *    │      │ │               │      │               │ │
 *    ▼...   └─┼───────────────┘      └───────────────┴─┘
 *             │                                      ▲
 *             └──────────────────► ──────────────────┘
 *              running led idx
 */

namespace jacket {

template <class SpiMaster, class Output, size_t PANEL_ROWS = 2 , size_t PANEL_COLUMNS = 5,size_t LED_ROWS_PER_PANEL = 16, size_t LED_COLUMNS_PER_PANEL = 8>
class JacketBack : public modm::ColorGraphicDisplay
{
public:
    // JacketBack()
    // {
    // }

    JacketBack()
    {
        this->clear();
    }

    template <class SystemClock >
    void
    initialize()
    {
        this->leds.template initialize<SystemClock>();
        this->clear();
    }

    uint16_t
	getWidth() const override
	{ return PANEL_COLUMNS*LED_COLUMNS_PER_PANEL; }

	uint16_t
	getHeight() const override
	{ return PANEL_ROWS*LED_ROWS_PER_PANEL; }

	inline std::size_t
	getBufferWidth() const final
	{
		return PANEL_COLUMNS*LED_COLUMNS_PER_PANEL;
	}

	inline std::size_t
	getBufferHeight() const final
	{
		return PANEL_ROWS*LED_ROWS_PER_PANEL;
	}
    
    void
    clear() final
    {
        std::fill(this->buffer.begin(), this->buffer.end(), this->backgroundColor.color);
    }

    void
	update()
    {
        size_t led_idx = 0;
        // loop over panel columns
        for (size_t panel_column=0; panel_column < PANEL_COLUMNS; panel_column++)
        {
            // every second panel goes bottom to top
            auto y_reverse = bool(panel_column % 2);
            // loop over all rows
            for (size_t row=0; row < PANEL_ROWS*LED_ROWS_PER_PANEL; row++)
            {
                // if reversed, idx is at bottom 
                auto row_idx = y_reverse ? PANEL_ROWS*LED_ROWS_PER_PANEL-1-row : row;
                
                // every second row goes right to left
                auto x_reverse = bool(row_idx % 2);
                
                // loop over columns of leds
                for (size_t led_column=0; led_column < LED_COLUMNS_PER_PANEL; led_column++)
                {
                    // if reversed, idx starts right
                    auto led_column_idx = x_reverse ? LED_COLUMNS_PER_PANEL-1-led_column : led_column;
                    auto x = panel_column*LED_COLUMNS_PER_PANEL + led_column_idx;
                    auto y = row_idx;
                    auto coord = std::make_pair(x,y);
                    // exclude this pixel if in mask
                    if (std::find(this->mask.begin(), this->mask.end(), coord)!=this->mask.end())
                    {
                        continue;
                    }
                    else
                    {
                        // compute index in buffer
                        auto idx_buf = y*PANEL_COLUMNS*LED_COLUMNS_PER_PANEL + x;
                        leds.setColor(led_idx, modm::color::Rgb(this->buffer.at(idx_buf)));
                        // increment the actual index
                        led_idx++;
                    }
                }
            }
        }
	}
    
    void
	setPixel(int16_t x, int16_t y) final
	{
		if (x < 0 or getWidth() <= x or y < 0 or getHeight() <= y) return;
		buffer[y * getWidth() + x] = this->foregroundColor.color;
	}

	void
	clearPixel(int16_t x, int16_t y) final
	{
		if (x < 0 or getWidth() <= x or y < 0 or getHeight() <= y) return;
		buffer[y * getWidth() + x] = this->backgroundColor.color;
	}

	modm::color::Rgb565
	getPixel(int16_t x, int16_t y) const final
	{
		if (x < 0 or getWidth() <= x or y < 0 or getHeight() <= y) return false;
		return buffer[y * getWidth() + x];
	}

    // LEDs buffer (we will draw on this)
    // static constexpr size_t led_rows_per_panel = 32;
    // static constexpr size_t led_columns_per_panel = 8;
    // static constexpr size_t panel_rows = 2;
    // static constexpr size_t panel_columns = 5;
    // static constexpr size_t led_buf_size = panel_rows_back*led_rows_per_panel*columns_back*led_columns_per_panel;
    std::array<uint16_t, PANEL_ROWS*LED_ROWS_PER_PANEL*PANEL_COLUMNS*LED_COLUMNS_PER_PANEL> buffer; // row major

    // mask of x,y coordinate pairs which are not on the jacket
    static constexpr auto mask = std::to_array<std::pair<size_t, size_t>>({
          {0,0},{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0},
		  {0,1},{1,1},{2,1},{3,1},{4,1},{5,1},{6,1},
		  {0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{6,2},
		  {0,3},{1,3},{2,3},{3,3},{4,3},{5,3},
		  {0,4},{1,4},{2,4},{3,4},{4,4},{5,4},
		  {0,5},{1,5},{2,5},{3,5},{4,5},
		  {0,6},{1,6},{2,6},{3,6},{4,6},
		  {0,7},{1,7},{2,7},{3,7},
		  {0,8},{1,8},{2,8},{3,8},
		  {0,9},{1,9},{2,9},
		  {0,10},{1,10},{2,10},
		  {0,11},{1,11},
		  {0,12},{1,12},
		  {0,13},
		  {0,13},
		  {32,0},{33,0},{34,0},{35,0},{36,0},{37,0},{38,0},{39,0},
		  		 {33,1},{34,1},{35,1},{36,1},{37,1},{38,1},{39,1},
		  		 {33,2},{34,2},{35,2},{36,2},{37,2},{38,2},{39,2},
		  			    {34,3},{35,3},{36,3},{37,3},{38,3},{39,3},
		  			    {34,4},{35,4},{36,4},{37,4},{38,4},{39,4},
		  			  		   {35,5},{36,5},{37,5},{38,5},{39,5},
		  			  		   {35,6},{36,6},{37,6},{38,6},{39,6},
		  			  			      {36,7},{37,7},{38,7},{39,7},
		  			  			      {36,8},{37,8},{38,8},{39,8},
		  			  			  		     {37,9},{38,9},{39,9},
		  			  			  	         {37,10},{38,10},{39,10},
		  			  			  			         {38,11},{39,11},
		  			  			  			         {38,12},{39,12},
		  			  			  			   	     {39,13},
		  			  			  			   		 {39,13}
    });
    
    modm::Ws2812b<SpiMaster, Output, PANEL_ROWS*LED_ROWS_PER_PANEL*PANEL_COLUMNS*LED_COLUMNS_PER_PANEL-mask.size()> leds;
};
}