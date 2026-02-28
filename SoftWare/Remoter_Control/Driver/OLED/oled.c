#include "oled.h"
#include "oledfont.h"
#include "stdlib.h"

u8 OLED_GRAM[128][8];
void OLED_Refresh_Gram(void) {
  u8 i, n;
  for (i = 0; i < 8; i++) {
    OLED_WR_Byte(0xb0 + i, OLED_CMD); // ����ҳ��ַ��0~7��
    OLED_WR_Byte(0x00, OLED_CMD);     // ������ʾλ�á��е͵�ַ
    OLED_WR_Byte(0x10, OLED_CMD);     // ������ʾλ�á��иߵ�ַ
    for (n = 0; n < 128; n++)
      OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
  }
}

// ��OLEDд��һ���ֽڡ�
// dat:Ҫд�������/����
// cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(u8 dat, u8 cmd) {
  u8 i;
  if (cmd)
    OLED_RS_Set();
  else
    OLED_RS_Clr();
  for (i = 0; i < 8; i++) {
    OLED_SCLK_Clr();
    if (dat & 0x80)
      OLED_SDIN_Set();
    else
      OLED_SDIN_Clr();
    OLED_SCLK_Set();
    dat <<= 1;
  }
  OLED_RS_Set();
}

// ����OLED��ʾ
void OLED_Display_On(void) {
  OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC����
  OLED_WR_Byte(0X14, OLED_CMD); // DCDC ON
  OLED_WR_Byte(0XAF, OLED_CMD); // DISPLAY ON
}
// �ر�OLED��ʾ
void OLED_Display_Off(void) {
  OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC����
  OLED_WR_Byte(0X10, OLED_CMD); // DCDC OFF
  OLED_WR_Byte(0XAE, OLED_CMD); // DISPLAY OFF
}
// ��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!
void OLED_Clear(void) {
  u8 i, n;
  for (i = 0; i < 8; i++) {
    for (n = 0; n < 128; n++) {
      OLED_GRAM[n][i] = 0X00;
    }
  }
  OLED_Refresh_Gram(); // ������ʾ
}
// ����
// x:0~127
// y:0~63
// t:1 ��� 0,���
void OLED_DrawPoint(u8 x, u8 y, u8 t) {
  u8 pos, bx, temp = 0;
  if (x > 127 || y > 63)
    return; // ������Χ��.
  pos = 7 - y / 8;
  bx = y % 8;
  temp = 1 << (7 - bx);
  if (t)
    OLED_GRAM[x][pos] |= temp;
  else
    OLED_GRAM[x][pos] &= ~temp;
}

// ��ָ��λ����ʾһ���ַ�,���������ַ�
// x:0~127
// y:0~63
// mode:0,������ʾ;1,������ʾ
// size:ѡ������ 16/12
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode) {
  u8 temp, t, t1;
  u8 y0 = y;
  chr = chr - ' '; // �õ�ƫ�ƺ��ֵ
  for (t = 0; t < size; t++) {
    if (size == 12)
      temp = oled_asc2_1206[chr][t]; // ����1206����
    else
      temp = oled_asc2_1608[chr][t]; // ����1608����
    for (t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80)
        OLED_DrawPoint(x, y, mode);
      else
        OLED_DrawPoint(x, y, !mode);
      temp <<= 1;
      y++;
      if ((y - y0) == size) {
        y = y0;
        x++;
        break;
      }
    }
  }
}
// ��ʾ����
void OLED_ShowCH(u8 x, u8 y, u8 chr, u8 size, u8 mode) {
  u8 temp, t, t1;
  u8 y0 = y;

  for (t = 0; t < size; t++) {
    temp = oled_CH_1616[chr][t]; // ���������ֿ�
    for (t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80)
        OLED_DrawPoint(x, y, mode);
      else
        OLED_DrawPoint(x, y, !mode);
      temp <<= 1;
      y++;
      if ((y - y0) == size) {
        y = y0;
        x++;
        break;
      }
    }
  }
}

// ��ʾ���������ַ�
void OLED_Show_CH(u8 x, u8 y, u8 chr, u8 size, u8 mode) {
  OLED_ShowCH(x, y, chr * 2, size, 1);
  OLED_ShowCH(x + size / 2, y, chr * 2 + 1, size, 1);
}

// ��ʾһ�������ַ�
void OLED_Show_CH_String(u8 x, u8 y, const u8 *p, u8 size, u8 mode) {
  u8 temp, t, t1;
  u8 y0 = y;

  for (t = 0; t < size; t++) {
    temp = p[t]; // ���������ֿ�
    for (t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80)
        OLED_DrawPoint(x, y, mode);
      else
        OLED_DrawPoint(x, y, !mode);
      temp <<= 1;
      y++;
      if ((y - y0) == size) {
        y = y0;
        x++;
        break;
      }
    }
  }
}
// m^n����
u32 oled_pow(u8 m, u8 n) {
  u32 result = 1;
  while (n--)
    result *= m;
  return result;
}
// ��ʾ2������
// x,y :�������
// len :���ֵ�λ��
// size:�����С
// mode:ģʽ	0,���ģʽ;1,����ģʽ
// num:��ֵ(0~4294967295);
void OLED_ShowNumber(u8 x, u8 y, u32 num, u8 len, u8 size) {
  u8 t, temp;
  u8 enshow = 0;
  for (t = 0; t < len; t++) {
    temp = (num / oled_pow(10, len - t - 1)) % 10;
    if (enshow == 0 && t < (len - 1)) {
      if (temp == 0) {
        OLED_ShowChar(x + (size / 2) * t, y, ' ', size, 1);
        continue;
      } else
        enshow = 1;
    }
    OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, 1);
  }
}
// ��ʾ�ַ���
// x,y:�������
//*p:�ַ�����ʼ��ַ
// ��16����
void OLED_ShowString(u8 x, u8 y, const u8 *p, u8 size, u8 mode) {
#define MAX_CHAR_POSX 122
#define MAX_CHAR_POSY 58
  while (*p != '\0') {
    if (x > MAX_CHAR_POSX) {
      x = 0;
      y += 16;
    } // ����
    if (y > MAX_CHAR_POSY) {
      y = x = 0;
      OLED_Clear();
    }
    OLED_ShowChar(x, y, *p, size, mode);
    x += 8;
    p++;
  }
}
// ��ʾ�ַ���
// x,y:�������
//*p:�ַ�����ʼ��ַ
// ��16����
void OLED_ShowString_16(u8 x, u8 y, const u8 *p) {
#define MAX_CHAR_POSX 122
#define MAX_CHAR_POSY 58
  while (*p != '\0') {
    if (x > MAX_CHAR_POSX) {
      x = 0;
      y += 16;
    } // ����
    if (y > MAX_CHAR_POSY) {
      y = x = 0;
      OLED_Clear();
    }
    OLED_ShowChar(x, y, *p, 16, 1);
    x += 8;
    p++;
  }
}

// ��ʼ��OLED
void OLED_Init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // ʹ��PB�˿�ʱ��

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  //	//�ı�ָ���ܽŵ�ӳ�� GPIO_Remap_SWJ_Disable SWJ ��ȫ���ã�JTAG+SW-DP��
  //	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
  //	//�ı�ָ���ܽŵ�ӳ�� GPIO_Remap_SWJ_JTAGDisable ��JTAG-DP ���� + SW-DP
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // �������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // �ٶ�50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);            // ��ʼ��PA0,1
  GPIO_SetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_15);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // �������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // �ٶ�50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);            // ��ʼ��PA2
  GPIO_SetBits(GPIOB, GPIO_Pin_14);

  //   OLED_RST_Clr();
  //   Delay(1000);
  OLED_RST_Set();
  Delay(200);
  OLED_RST_Clr();
  Delay(200);
  OLED_RST_Set();
  Delay(200);
  OLED_WR_Byte(0xAE, OLED_CMD); //--turn off oled panel
  OLED_WR_Byte(0x00, OLED_CMD); //---set low column address
  OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
  OLED_WR_Byte(0x40, OLED_CMD); //--set start line address  Set Mapping RAM
                                // Display Start Line (0x00~0x3F)
  OLED_WR_Byte(0x81, OLED_CMD); //--set contrast control register
  OLED_WR_Byte(0xFF, OLED_CMD); // Set SEG Output Current Brightness
  OLED_WR_Byte(0xA1,
               OLED_CMD); //--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
  OLED_WR_Byte(0xC8,
               OLED_CMD); // Set COM/Row Scan Direction   0xc0���·��� 0xc8����
  OLED_WR_Byte(0xA6, OLED_CMD); //--set normal display
  OLED_WR_Byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
  OLED_WR_Byte(0x3f, OLED_CMD); //--1/64 duty
  OLED_WR_Byte(
      0xD3,
      OLED_CMD); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
  OLED_WR_Byte(0x00, OLED_CMD); //-not offset
  OLED_WR_Byte(
      0xd5, OLED_CMD); //--set display clock divide ratio/oscillator frequency
  OLED_WR_Byte(0x80,
               OLED_CMD); //--set divide ratio, Set Clock as 100 Frames/Sec
  OLED_WR_Byte(0xD9, OLED_CMD); //--set pre-charge period
  OLED_WR_Byte(0xF1,
               OLED_CMD); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  OLED_WR_Byte(0xDA, OLED_CMD); //--set com pins hardware configuration
  OLED_WR_Byte(0x12, OLED_CMD);
  OLED_WR_Byte(0xDB, OLED_CMD); //--set vcomh
  OLED_WR_Byte(0x40, OLED_CMD); // Set VCOM Deselect Level
  OLED_WR_Byte(0x20, OLED_CMD); //-Set Page Addressing Mode (0x00/0x01/0x02)
  OLED_WR_Byte(0x02, OLED_CMD); //
  OLED_WR_Byte(0x8D, OLED_CMD); //--set Charge Pump enable/disable
  OLED_WR_Byte(0x14, OLED_CMD); //--set(0x10) disable
  OLED_Clear();
  OLED_WR_Byte(0xAF, OLED_CMD); // 开启显示

  // 测试显示
  //   OLED_ShowString(0, 0, "OLED TEST", 16, 1);
  //   OLED_ShowString(0, 16, "Hello!", 16, 1);

  for (u8 i = 0; i < 8; i++) {
    OLED_WR_Byte(0xb0 + i, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    for (u8 j = 0; j < 128; j++)
      OLED_WR_Byte(0xFF, OLED_DATA); // 全屏点亮测试
  }
  Show.windows = 0;
}
