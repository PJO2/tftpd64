// source released under artistic license (see license.txt)
//


const struct S_LVHeader
{
    int			fmt;		// format (cf LVCOLUMN)
    int			cx;			// width
    char		*pszText;	// title
} ;

void SetViewMode (HWND hwndLV, DWORD dwView, DWORD dwExtStyle);
BOOL InitTftpd32ListView (HWND hListV, const struct S_LVHeader *tCol, int Nb, DWORD swExtStyle);
LRESULT ProcessCustomDraw (LPARAM lParam);

int CALLBACK CompareStringFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);


