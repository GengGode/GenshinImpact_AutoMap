#include "GenshinImpact_AutoMap.h"

giam::GenshinImpact_AutoMap::GenshinImpact_AutoMap(){}

giam::GenshinImpact_AutoMap::~GenshinImpact_AutoMap(){}

//初始化
bool giam::GenshinImpact_AutoMap::init()
{
	//AT.on();
	
	isInit = false;
	isRun = true;
	//初始化背景
	autoMapMat = Mat(autoMapSize, CV_8UC4, Scalar(200, 200, 200, 255));
	//创建窗口
	namedWindow(autoMapWindowsName);
	//设置回调
	setMouseCallback(autoMapWindowsName, on_MouseHandle, (void*)this);
	//获取悬浮窗句柄
	thisHandle = FindWindowA(NULL, autoMapWindowsName.c_str());
	//初始化绘图
	imshow(autoMapWindowsName, autoMapMat);
	//设置窗口位置
	SetWindowPos(thisHandle, HWND_TOPMOST, giRect.left + offGiMinMap.x, giRect.top + offGiMinMap.y, autoMapSize.width, autoMapSize.height, SWP_NOMOVE);
	//设置窗口为无边框
	SetWindowLong(thisHandle, GWL_STYLE, GetWindowLong(thisHandle, GWL_EXSTYLE | WS_EX_TOPMOST)); //改变窗口风格
	ShowWindow(thisHandle, SW_SHOW);

	return false;
}

//运行
bool giam::GenshinImpact_AutoMap::run()
{
	//运行框架
	if (isInit) { init(); }
	//运行循环
	while (isRun)
	{
		customProcess();
		//地图图标文字等更新
		mapUpdata();
		//显示
		mapShow();
	}
	//退出处理
	exit();
	return false;
}

//退出
bool giam::GenshinImpact_AutoMap::exit()
{
	return false;
}

//从大地图中截取显示区域
Mat giam::GenshinImpact_AutoMap::getMinMap()
{
	Mat minMap;

	Point minMapPoint = Point(0, 0);

	Size reMapSize = autoMapSize;
	reMapSize.width = (int)(reMapSize.width * giMEF.scale);
	reMapSize.height = (int)(reMapSize.height * giMEF.scale);

	Size R = reMapSize / 2;

	Point LT = zerosMinMap - Point(R);
	Point RB = zerosMinMap + Point(R);

	minMapPoint = LT;

	if (LT.x < 0)
	{
		minMapPoint.x = 0;
	}
	if (LT.y < 0)
	{
		minMapPoint.y = 0;
	}
	if (RB.x > mapSize.width)
	{
		minMapPoint.x = mapSize.width- reMapSize.width;
	}
	if (RB.y > mapSize.height)
	{
		minMapPoint.y = mapSize.height - reMapSize.height;
	}
	minMapRect = Rect(minMapPoint, reMapSize);

	resize(mapMat(minMapRect), minMap, autoMapSize);
	//minMap = mapMat(Rect(minMapPoint, reMapSize));

	return minMap;
}

//判断RECT是否相等
bool giam::GenshinImpact_AutoMap::isEqual(RECT & r1, RECT & r2)
{
	if (r1.bottom != r2.bottom || r1.left != r2.left || r1.right != r2.right || r1.top != r2.top)
	{
		return false;
	}
	else
	{
		return true;
	}
	return false;
}

//判断RECT是否包含Point
bool giam::GenshinImpact_AutoMap::isContains(RECT & r, Point & p)
{
	if (p.x<r.left || p.x>r.right||p.y<r.top||p.y>r.bottom)
	{
		return false;
	}
	else
	{
		return true;
	}

	return false;
}

//判断Rect是否包含Point
bool giam::GenshinImpact_AutoMap::isContains(Rect & r, Point & p)
{
	if (p.x<r.x || p.x>(r.x+r.width) || p.y<r.y || p.y>(r.y+r.height))
	{
		return false;
	}
	else
	{
		return true;
	}
	return false;
}

//原神是否运行
void giam::GenshinImpact_AutoMap::giIsRunning()
{
	//尝试获取原神句柄
	giHandle = FindWindowA(NULL, "原神");
	if (giHandle != NULL)
	{
		giIsRunningFlag = true;
		return;
	}
	giIsRunningFlag= false;
}

//原神是否可见
void giam::GenshinImpact_AutoMap::giIsDisplay()
{
	if (giHandle != NULL)
	{
		giIsDisplayFlag = !IsIconic(giHandle);
		return;
	}
	giIsDisplayFlag = false;
}

//原神是否最大化
void giam::GenshinImpact_AutoMap::giIsZoomed()
{
	if (giHandle != NULL)
	{
		giIsZoomedFlag = IsZoomed(giHandle);
		//获取原神窗口区域
		GetWindowRect(giHandle, &giRect);
		return;
	}
	giIsZoomedFlag = false;
}

//检查原神窗口状态
void giam::GenshinImpact_AutoMap::giCheckWindows()
{
	giIsRunning();
	giIsDisplay();
	giIsZoomed();
}

//获取原神画面
void giam::GenshinImpact_AutoMap::giGetScreen()
{
	HBITMAP	hBmp;
	RECT rc;
	BITMAP bmp;

	if (giHandle == NULL)return;

	//获取目标句柄的窗口大小RECT
	GetWindowRect(giHandle, &rc);

	//获取目标句柄的DC
	HDC hScreen = GetDC(giHandle);
	HDC	hCompDC = CreateCompatibleDC(hScreen);

	//获取目标句柄的宽度和高度
	int	nWidth = rc.right - rc.left;
	int	nHeight = rc.bottom - rc.top;

	//创建Bitmap对象
	hBmp = CreateCompatibleBitmap(hScreen, nWidth, nHeight);//得到位图

	SelectObject(hCompDC, hBmp); //不写就全黑
	BitBlt(hCompDC, 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);

	//释放对象
	DeleteDC(hScreen);
	DeleteDC(hCompDC);

	//类型转换
	GetObject(hBmp, sizeof(BITMAP), &bmp);

	int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel / 8;
	int depth = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;

	//mat操作
	giFrame.create(cv::Size(bmp.bmWidth, bmp.bmHeight), CV_MAKETYPE(CV_8U, nChannels));
	
	GetBitmapBits(hBmp, bmp.bmHeight*bmp.bmWidth*nChannels, giFrame.data);
}

//设置HUD
void giam::GenshinImpact_AutoMap::setHUD()
{
	if (giIsRunningFlag)
	{
		giHUD.runState = "Genshin Impact Is Running";
		giHUD.runTextColor = Scalar(0, 255, 0);

	}
	else
	{
		giHUD.runState = "Genshin Impact Not Run";
		giHUD.runTextColor = Scalar(0, 0, 255);
	};
	if (giIsDisplayFlag)
	{
		giHUD.displayFlagColor = Scalar(0, 255, 0);
	}
	else
	{
		giHUD.displayFlagColor = Scalar(0, 0, 255);
	}

}

//绘制HUD
void giam::GenshinImpact_AutoMap::addHUD(Mat img)
{
	Mat tmp;

	//绘制背景条
	Mat backRect(20, autoMapSize.width, CV_8UC4, Scalar(200, 200,200, 255));
	tmp = img(Rect(0,0, autoMapSize.width, 20));
	addWeighted(tmp, 0.6, backRect, 0.4, 0, tmp);

	//绘制中心光标
	//Mat star;
	//img(Rect(autoMapSize.width / 2 - 5, autoMapSize.height / 2 - 5, 11, 11)).copyTo(star);
	//tmp = img(Rect(autoMapSize.width / 2 - 5, autoMapSize.height / 2 - 5, 11, 11));
	//circle(star, Point(5, 5), 4, giHUD.minStarColor, 2, LINE_AA);
	//addWeighted(tmp, 0.5, star, 0.5, 0, tmp);

	//绘制小图标
	Mat backgound;
	tmp = img(giTab.pngARect);//Rect(30, 0, giTab.pngA.cols, giTab.pngA.rows)
	tmp.copyTo(backgound);
	giTab.pngA.copyTo(tmp, giTab.pngAMask);

	if (!giFlag.isShow[0])
	{
		//小图标半隐藏
		addWeighted(tmp, 0.5, backgound, 0.5, 0, tmp);

	}
	else
	{
		addWeighted(tmp, 0.7, backgound, 0.2, 0, tmp);

	}
	tmp.release();
	tmp = img(Rect(autoMapSize.width- giTab.sysIcon1.cols-10- giTab.sysIcon2.cols, 0, giTab.sysIcon2.cols, giTab.sysIcon2.rows));
	//tmp.copyTo(backgound);
	giTab.sysIcon2.copyTo(tmp, giTab.sysIcon2Mask);
	//addWeighted(tmp, 0.5, giTab.sysIcon1, 0.5, 0, tmp);

	tmp.release();
	tmp = img(Rect(autoMapSize.width - giTab.sysIcon1.cols , 0, giTab.sysIcon1.cols, giTab.sysIcon1.rows));
	//tmp.copyTo(backgound);
	giTab.sysIcon1.copyTo(tmp, giTab.sysIcon1Mask);


	//圆点显示原神状态
	//circle(img, Point(10, 10), 4, giHUD.displayFlagColor, -1);
	//circle(img, Point(20, 10), 4, giHUD.runTextColor, -1);

	//putText(img, giHUD.runState, Point(24, 12), FONT_HERSHEY_COMPLEX_SMALL, 0.4, giHUD.runTextColor, 1);

	//putText(img, to_string(giMEF.scale), Point(30, 14), FONT_HERSHEY_COMPLEX_SMALL, 0.4, giHUD.runTextColor, 1);

}

//设置标记是否显示
void giam::GenshinImpact_AutoMap::setFLAG()
{
	// mouse click change giFlag.isShow[] state

}

//在地图上绘制标记
void giam::GenshinImpact_AutoMap::addFLAG(Mat img)
{
	//更新Flag为真
	if (giFlag.isUpdata||giFlag.isGetMap)
	{
		for (int i = 0; i < giFlag.max; i++)
		{
			//显示Flag为真
			if (giFlag.isShow[i])
			{
				for (int j = 0; j < 3/*OBJ.obj1.object.*/; j++)
				{

					Point p = Point(OBJ.obj1.at(j).x, OBJ.obj1.at(j).y);
					//目标点在小地图显示区域内
					if (isContains(minMapRect, p))
					{
						Mat r;
						int x = (int)((p.x - minMapRect.x) / giMEF.scale);
						int y = (int)((p.y - minMapRect.y) / giMEF.scale);
						//该x，y左下角要有足够的空间来填充图标
						if (x + giTab.pngA.cols > autoMapSize.width || y + giTab.pngA.rows > autoMapSize.height)
						{
							;
						}
						else
						{
							r = img(Rect(x, y, giTab.pngA.cols, giTab.pngA.rows));
							giTab.pngA.copyTo(r, giTab.pngAMask);
						}

					}

				}
			}
		}


		
		giFlag.isUpdata = false;
	}

}

//测试用
void giam::GenshinImpact_AutoMap::customProcess()
{
	_count++;
	//giTab.HBitmap2Mat(giTab.aa, giTab.png);

}

//地图数据状态更新
void giam::GenshinImpact_AutoMap::mapUpdata()
{
	//更新用
	 Mat tmpMap;

	//更新原神窗口状态
	giCheckWindows();
	//获取原神窗口截屏
	//giGetScreen();

	//截取地图
	if (giFlag.isGetMap)
	{
		getMinMap().copyTo(tmpMap);
		giFlag.isGetMap = false;
		giFlag.isUpHUD = true;
	}

	//设置显示标记
	setFLAG();
	addFLAG(tmpMap);



	//设置显示HUD
	setHUD();
	if (giFlag.isUpHUD)
	{
		addHUD(tmpMap);

		//将加工好的画面赋给显示变量
		tmpMap.copyTo(autoMapMat);
		giFlag.isUpHUD = false;

	}

}

//地图显示刷新
void giam::GenshinImpact_AutoMap::mapShow()
{
	//如果悬浮窗窗口不存在，停止运行并退出
	if (IsWindow(thisHandle))
	{
		//如果窗口处于最小化状态，不对画面进行更新
		if (!IsIconic(thisHandle))
		{
			imshow(autoMapWindowsName, autoMapMat);
			if (offGiMinMap != offGiMinMapTmp)
			{
				SetWindowPos(thisHandle, HWND_TOPMOST, giRect.left + offGiMinMap.x, giRect.top + offGiMinMap.y, 0, 0, SWP_NOSIZE);
				offGiMinMapTmp = offGiMinMap;

			}
		}

		//如果原神正在运行
		if (giIsRunningFlag)
		{
			//并且处于可见状态
			if (giIsDisplayFlag)
			{
				//如果悬浮窗处于最小化，则恢复悬浮窗
				if (IsIconic(thisHandle))
				{
					ShowWindow(thisHandle, SW_RESTORE);
				}
				//如果原神窗口有移动，悬浮窗随之移动
				if (!isEqual(giRect, giRectTmp)|| offGiMinMap != offGiMinMapTmp)
				{
					SetWindowPos(thisHandle, HWND_TOPMOST, giRect.left + offGiMinMap.x, giRect.top + offGiMinMap.y, 0, 0, SWP_NOSIZE);
					giRectTmp = giRect;
					offGiMinMapTmp = offGiMinMap;
				}
			}
			else
			{
				//处于不可见状态则令悬浮窗进行最小化
				ShowWindow(thisHandle, SW_MINIMIZE);
			}
		}
		//等待显示更新
		FRL.Wait();
	}
	else
	{
		isRun = false;
	}
}

//鼠标回调
void giam::GenshinImpact_AutoMap::on_MouseHandle(int event, int x, int y, int flags, void * parm)
{
	GenshinImpact_AutoMap& gi = *(giam::GenshinImpact_AutoMap*) parm;

	gi.giMEF.value = flags;

	switch (event)	
	{
	case EVENT_MOUSEMOVE: 
	{
		break;
	}
	case EVENT_LBUTTONDOWN: 
	{
		gi.giMEF.x0 = x;
		gi.giMEF.y0 = y;
		gi.giMEF.p0 = gi.zerosMinMap;
		break;
	}
	case EVENT_RBUTTONDOWN:
	{
		break;
	}
	case EVENT_MBUTTONDOWN: 
	{
		gi.giMEF.x1 = x;
		gi.giMEF.y1 = y;
		gi.giMEF.p1 = gi.offGiMinMap;//Point(gi.offGiMinMap.x - x, gi.offGiMinMap.y - y);
		break;
	}
	case EVENT_LBUTTONUP: 
	{
		Rect& r = gi.giTab.pngARect;

		if (x<r.x || x>(r.x + r.width) || y<r.y || y>(r.y + r.height))
		{
			
		}
		else
		{
			gi.giFlag.isShow[0] = !gi.giFlag.isShow[0];
			gi.giFlag.isUpdata = true;
			gi.giFlag.isGetMap = true;
		}

		break;
	}
	case EVENT_RBUTTONUP: 
	{
		break;
	}
	case EVENT_MBUTTONUP: 
	{
		break;
	}
	case EVENT_LBUTTONDBLCLK: 
	{
		if (x > gi.autoMapSize.width - gi.giTab.sysIcon1.cols&&y <= gi.giTab.sysIcon1.rows)
		{
			gi.isRun = false;
		}
		break;
	}
	case EVENT_RBUTTONDBLCLK: 
	{

		break;
	}
	case EVENT_MBUTTONDBLCLK:
	{
		break;
	}
	case EVENT_MOUSEWHEEL:
	{
		gi.giMEF.value = getMouseWheelDelta(flags);

			if (gi.giMEF.value < 0)
			{
				if(gi.giMEF.scale < 6)
				{
					gi.giMEF.scale *= 1.2;
				}
			}
			else if (gi.giMEF.value > 0)
			{
				if (gi.giMEF.scale >0.5)
				{
					gi.giMEF.scale /= 1.2;
				}
			}
			gi.giFlag.isGetMap = true;
			gi.giFlag.isUpdata = true;
		break;
	}
	case EVENT_MOUSEHWHEEL:
	{
		break;
	}
	default:
		break;
	}

	switch (flags)
	{
	case EVENT_FLAG_LBUTTON:
	{
		if (x > gi.autoMapSize.width - gi.giTab.sysIcon1.cols - 10 - gi.giTab.sysIcon2.cols&&x < gi.autoMapSize.width - gi.giTab.sysIcon2.cols - 10 && y < gi.giTab.sysIcon1.rows&&y>0)
		{
			//gi.giMEF.dx = x - gi.giMEF.x1;
			//gi.giMEF.dy = y - gi.giMEF.y1;
			//gi.offGiMinMap = gi.giMEF.p1 + Point(gi.giMEF.dx, gi.giMEF.dy);
		}
		else
		{
			gi.giMEF.dx = x - gi.giMEF.x0;
			gi.giMEF.dy = y - gi.giMEF.y0;
			gi.zerosMinMap = gi.giMEF.p0 - Point((int)(gi.giMEF.dx*gi.giMEF.scale), (int)(gi.giMEF.dy*gi.giMEF.scale));
			gi.giFlag.isUpdata = true;
			gi.giFlag.isGetMap = true;
		}

		break;
	}
	case EVENT_FLAG_RBUTTON:
	{
		break;
	}
	case EVENT_FLAG_MBUTTON:
	{
		//gi.giMEF.dx = x - gi.giMEF.x1;
		//gi.giMEF.dy = y - gi.giMEF.y1;
		gi.offGiMinMap = gi.giMEF.p1 + Point(x - gi.giMEF.x1, y - gi.giMEF.y1);

		break;
	}
	case EVENT_FLAG_CTRLKEY:
	{
		break;
	}
	case EVENT_FLAG_SHIFTKEY:
	{
		break;
	}
	case EVENT_FLAG_ALTKEY:
	{
		break;
	}
	default:
		break;
	}
}
