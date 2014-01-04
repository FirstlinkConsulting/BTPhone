/**
 * @file
 * @brief ANYKA software
 * This file is for windows graphic operation
 *
 * @author Justin.Zhao
 * @date 2008-3-31
 * @version 1.0
 */

#include "Eng_Graph.h"
#include "Eng_Font.h"

#if(NO_DISPLAY == 0)

#pragma arm section zidata = "_bootbss_"
T_GRAPH g_Graph;
#pragma arm section zidata

T_BOOL Eng_PtInRect(const T_RECT *rect, T_POINT *pt)
{
	if(pt->x>=rect->left && pt->x<(rect->left+rect->width))
	{
		if(pt->y>=rect->top && pt->y<(rect->top+rect->height))
		{
			return AK_TRUE;
		}
	}
	return AK_FALSE;
}

#endif
