#include "gui.h"

//current list element
GUI_ListData *gui_CurList;

//global vars for message
char *GUI_MessageText;
void (*GUI_MessageDraw)();
uint8_t (*GUI_MessageInput)(uint8_t);

//current app 
void (*GUI_curapp_draw)();
uint8_t (*GUI_curapp_input)(uint8_t);


bool gui_init()
{
	gui_CurList = 0;
	return SSD1306_Init();
}

void gui_text(const char* txt, uint8_t x, uint8_t y, uint8_t col)
{
	SSD1306_GotoXY(x, y + 1);
	SSD1306_Puts(txt, &GUI_DefFont, col);
}

void gui_set_cur_app(void (*appDraw)(), uint8_t (*appInput)(uint8_t))
{
	GUI_curapp_draw = appDraw;
	GUI_curapp_input = appInput;
}

void gui_lable(const char* txt, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t bg, uint8_t border)
{
	SSD1306_DrawFilledRectangle(x, y, w, h, bg);
	if(border)
		SSD1306_DrawRectangle(x, y, w, h, !bg);
	uint8_t max_x = x + border;
	SSD1306_GotoXY(x+border, y+border + (h - GUI_DefFont.FontHeight)/2 + 1);
	while(*txt && max_x + GUI_DefFont.FontWidth < SSD1306_WIDTH && max_x + GUI_DefFont.FontWidth < x + w)
	{
		max_x += GUI_DefFont.FontWidth;
		SSD1306_Putc(*txt, &GUI_DefFont, !bg);
		txt++;
	}
}

void gui_lable_multiline(const char* txt, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t bg, uint8_t border)
{
	SSD1306_DrawFilledRectangle(x, y, w, h, bg);
	uint8_t max_x = x + border, cy = y + border;
	SSD1306_GotoXY(x+border, cy);
	while(*txt)
	{
		if(max_x + GUI_DefFont.FontWidth > SSD1306_WIDTH || max_x + GUI_DefFont.FontWidth > x + w || *txt == '\n' || *txt == '\r')
		{
			cy += GUI_DefFont.FontHeight;
			SSD1306_GotoXY(x+border, cy);
			max_x = x + border;
			if(cy + GUI_DefFont.FontHeight > y + h - border)
			{
				if(border)
					SSD1306_DrawRectangle(x, y, w, h, !bg);
				return;
			}
		}
		max_x += GUI_DefFont.FontWidth;
		if(*txt != '\n' && *txt != '\r')
			SSD1306_Putc(*txt, &GUI_DefFont, !bg);
		txt++;
	}
	if(border)
		SSD1306_DrawRectangle(x, y, w, h, !bg);
}

void gui_ticker(GUI_TickerData *dt)
{
	uint8_t maxlen = (dt->w - dt->border*2)/GUI_DefFont.FontWidth, len = 0;
	int	shift = 0;
	while(*(dt->text + len) != 0)
		len++;
	if(dt->startTick == 0)
	{
		dt->startTick = HAL_GetTick();
	}
	shift = ((HAL_GetTick() - dt->startTick) / GUI_TickerSpeed) 
		- maxlen/2; //start delay
	
	if(shift > len-maxlen)
	{
		if(shift > len-maxlen + GUI_TickerEndDelay)
		{
			shift = 0;
			dt->startTick = HAL_GetTick();
		}
		else
			shift = len-maxlen;
	}
	if(shift < 0)
	{
		shift = 0;
	}
	gui_lable(dt->text + shift, dt->x, dt->y, dt->w, dt->h, dt->bg, dt->border);
}
GUI_TickerData* gui_ticker_create(char *text, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t bg, uint8_t border)
{
	GUI_TickerData *data = (GUI_TickerData*)malloc(sizeof(GUI_TickerData));
	data->text = text;
	data->x = x;
	data->y = y;
	data->w = w;
	data->h = h;
	data->bg = bg;
	data->border = border;
	data->startTick = 0;
	return data;
}
void gui_removeTicker(GUI_TickerData *dt)
{
//	free(dt->text);
	free(dt);
}

void gui_set_curList(GUI_ListData* list)
{
	gui_CurList = list;
}
uint8_t gui_draw_list()
{
	if(gui_CurList == 0)
		return 0;
	SSD1306_DrawFilledRectangle(gui_CurList->x, gui_CurList->y, gui_CurList->w, gui_CurList->h, 0);
	SSD1306_DrawRectangle(gui_CurList->x, gui_CurList->y, gui_CurList->w, gui_CurList->h, 1);
	
	uint8_t ry = gui_CurList->y + 2;
	if(gui_CurList->header != 0)
	{
		gui_text(gui_CurList->header, gui_CurList->x + 1, gui_CurList->y, 1);
		ry += GUI_DefFont.FontHeight;
	}
	
	uint8_t maxC = ((gui_CurList->h - 3) / GUI_DefFont.FontHeight) - (gui_CurList->header != 0);
	
	uint16_t i;
	if(maxC >= gui_CurList->ItemsCount)
	{
		for(i = 0; i < gui_CurList->ItemsCount; i++)
		{
			if(i != gui_CurList->selectedItem)
				gui_lable(gui_CurList->items[i].text, gui_CurList->x + 1, ry + i*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 0, 0);
			else
				gui_lable(gui_CurList->items[i].text, gui_CurList->x +1 , ry + i*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 1, 0);
		}
	}
	else
	{
		if(gui_CurList->ItemsCount - 1 - gui_CurList->selectedItem < maxC / 2)
		{
			for(i = gui_CurList->ItemsCount - maxC; i < gui_CurList->ItemsCount; i++)
			{
				if(i != gui_CurList->selectedItem)
					gui_lable(gui_CurList->items[i].text, gui_CurList->x + 1, ry + (i - gui_CurList->ItemsCount + maxC)*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 0, 0);
				else
					gui_lable(gui_CurList->items[i].text, gui_CurList->x + 1, ry + (i - gui_CurList->ItemsCount + maxC)*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 1, 0);
			}
		}
		else if(gui_CurList->selectedItem < maxC / 2)
		{
			for(i = 0; i < maxC; i++)
			{
				if(i != gui_CurList->selectedItem)
					gui_lable(gui_CurList->items[i].text, gui_CurList->x + 1, ry + i*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 0, 0);
				else
					gui_lable(gui_CurList->items[i].text, gui_CurList->x + 1, ry + i*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 1, 0);
			}
		}
		else
		{
			for(i = gui_CurList->selectedItem - maxC/2; i < gui_CurList->selectedItem - maxC/2 + maxC; i++)
			{
				if(i != gui_CurList->selectedItem)
					gui_lable(gui_CurList->items[i].text, gui_CurList->x + 1, ry + (i - gui_CurList->selectedItem + maxC/2)*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 0, 0);
				else
					gui_lable(gui_CurList->items[i].text, gui_CurList->x + 1, ry + (i - gui_CurList->selectedItem + maxC/2)*GUI_DefFont.FontHeight, gui_CurList->w - 3, GUI_DefFont.FontHeight, 1, 0);
			}
		}
	}
	uint8_t sli_h = gui_CurList->h / gui_CurList->ItemsCount;
	if(sli_h < 10)
		sli_h = 10;
	uint8_t yy = ((gui_CurList->h) * gui_CurList->selectedItem) / gui_CurList->ItemsCount;
	SSD1306_DrawLine(gui_CurList->x, ry - 2,  gui_CurList->x + gui_CurList->w, ry - 2, 1);
	SSD1306_DrawLine(gui_CurList->x + gui_CurList->w - 1, gui_CurList->y + yy,  gui_CurList->x + gui_CurList->w - 1, gui_CurList->y + yy + sli_h, 1);
	return 0;
}


void gui_showMessage(char* text)
{
	if(GUI_MessageText != 0)
		free(GUI_MessageText);
	GUI_MessageText = text;
	GUI_MessageDraw = 0;
	GUI_MessageInput = 0;
}
void gui_showCustomMessage(void (*drawmsg)(), uint8_t (*msginput)(uint8_t))
{
	GUI_MessageText = 0;
	GUI_MessageDraw = drawmsg;
	GUI_MessageInput = msginput;
}
void gui_closeMessage(void)
{
	GUI_MessageText = 0;
	GUI_MessageDraw = 0;
	GUI_MessageInput = 0;
}
uint8_t gui_draw_message()
{
	if(GUI_MessageText == 0 && GUI_MessageInput == 0 && GUI_MessageDraw == 0)
		return 0;
	if(GUI_MessageDraw != 0)
	{
		GUI_MessageDraw();
		return 1;
	}
	SSD1306_DrawRectangle(0, 0, 126, 64, 1);
	gui_lable_multiline(GUI_MessageText, 2, 2, 122, 60, 0, 1);
	return 1;
}
uint8_t gui_input_message(uint8_t key)
{
	if(GUI_MessageText == 0 && GUI_MessageInput == 0 && GUI_MessageDraw == 0)
		return 0;
	if(GUI_MessageInput != 0)
	{
		if(GUI_MessageInput(key))
		{
			return 1;
		}
	}
	gui_closeMessage();
	return 1;
}

void gui_upd_display()
{
	SSD1306_UpdateScreen();
	SSD1306_Fill(SSD1306_COLOR_BLACK);
}
void gui_draw(void)
{
	if(gui_draw_message())
	{
		gui_upd_display();
		return;
	}
	if(GUI_curapp_draw != 0)
		GUI_curapp_draw();
	gui_draw_list();
	gui_upd_display();
}
