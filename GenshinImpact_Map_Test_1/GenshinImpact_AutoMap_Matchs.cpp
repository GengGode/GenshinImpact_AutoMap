#include "GenshinImpact_AutoMap_Matchs.h"

GenshinImpact_AutoMap_Matchs::GenshinImpact_AutoMap_Matchs()
{

}

GenshinImpact_AutoMap_Matchs::~GenshinImpact_AutoMap_Matchs()
{
}

GenshinImpact_AutoMap_Matchs::GenshinImpact_AutoMap_Matchs(Mat _target)
{
	setTarget(_target);


	//orb = ORB::create(10000, 1.6, 8, 31, 0, 2, ORB::HARRIS_SCORE, 31, 20);
	//通过ORB算法检测两幅图像中的特征点，并计算各自的二值描述子
	//orb->detectAndCompute(target, Mat(), keyPoints_test, descriptors_test, false);

#if 0

	namedWindow("2", 256);
	imshow("2", descriptors_scene);
#endif
}

void GenshinImpact_AutoMap_Matchs::init()
{
	if (isInit) return;

	detector = cv::xfeatures2d::SURF::create(minHessian);
	detector->detectAndCompute(target, noArray(), keypoints_scene, descriptors_scene);

	isInit = true;
}

void GenshinImpact_AutoMap_Matchs::setMode(int flag)
{
	mode = flag;
}

void GenshinImpact_AutoMap_Matchs::setObject(Mat img)
{
	object = img;
	isObjectExist = true;
}

void GenshinImpact_AutoMap_Matchs::setTarget(Mat img)
{
	target = img;
	isTargetExist = true;
}

void GenshinImpact_AutoMap_Matchs::setRectUID(Mat img)
{
	rectUID = img;
	isRectUIDExist = true;
}

void GenshinImpact_AutoMap_Matchs::getKeyPoints()
{
	getObjectKeyPoints();
	getTargetKeyPoints();
}

void GenshinImpact_AutoMap_Matchs::onMatch()
{
}

Point GenshinImpact_AutoMap_Matchs::getLocation()
{
	return p;
}

void GenshinImpact_AutoMap_Matchs::testSURF()
{
	//surf
	Mat img_scene = target;
	Mat img_object = object;

	isCout = true;
	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "SURF Start time:" << t << "s" << endl;
		t = (double)cv::getTickCount();
	}

	detector->detectAndCompute(img_object, noArray(), keypoints_object, descriptors_object);


	//-- Step 2: Matching descriptor vectors with a FLANN based matcher
	// Since SURF is a floating-point descriptor NORM_L2 is used
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
	std::vector< std::vector<DMatch> > knn_matches;
	matcher->knnMatch(descriptors_object, descriptors_scene, knn_matches, 2);

	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "matcher->knnMatch time:" << t << "s" << endl;
		t = (double)cv::getTickCount();
	}

	//-- Filter matches using the Lowe's ratio test
	//const float ratio_thresh = 0.8f;
	//std::vector<DMatch> good_matches;
	std::vector<double> lisx;
	std::vector<double> lisy;
	double sumx = 0;
	double sumy = 0;
	for (size_t i = 0; i < knn_matches.size(); i++)
	{
		if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
		{
			//good_matches.push_back(knn_matches[i][0]);
			lisx.push_back(((img_object.cols / 2 - keypoints_object[knn_matches[i][0].queryIdx].pt.x)*1.3 + keypoints_scene[knn_matches[i][0].trainIdx].pt.x));
			lisy.push_back(((img_object.rows / 2 - keypoints_object[knn_matches[i][0].queryIdx].pt.y)*1.3 + keypoints_scene[knn_matches[i][0].trainIdx].pt.y));
			sumx += lisx.back();
			sumy += lisy.back();
		}
	}

	if (min(lisx.size(), lisy.size()) == 0)
	{
		cout << "SURF Match Fail" << endl;
		return;
	}
	cout<< "SURF Match Point Number: " << lisx.size()<<","<<lisy.size() << endl;
	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "good_matches.push_back time:" << t << "s" << endl;
		t = (double)cv::getTickCount();
	}

	double meanx = sumx / lisx.size(); //均值
	double meany = sumy / lisy.size(); //均值
	int x = (int)meanx;
	int y = (int)meany;
	if (min(lisx.size(), lisy.size()) > 15)
	{
		double accumx = 0.0;
		double accumy = 0.0;
		for (int i = 0; i < min(lisx.size(), lisy.size()); i++)
		{
			accumx += (lisx[i] - meanx)*(lisx[i] - meanx);
			accumy += (lisy[i] - meany)*(lisy[i] - meany);
		}

		double stdevx = sqrt(accumx / (lisx.size() - 1)); //标准差
		double stdevy = sqrt(accumy / (lisy.size() - 1)); //标准差

		sumx = 0;
		sumy = 0;
		int numx = 0;
		int numy = 0;
		for (int i = 0; i < min(lisx.size(), lisy.size()); i++)
		{
			if (abs(lisx[i] - meanx) < 3 * stdevx)
			{
				sumx += lisx[i];
				numx++;
			}
			if (abs(lisy[i] - meany) < 3 * stdevy)
			{
				sumy += lisy[i];
				numy++;
			}
		}
		int x = (int)(sumx / numx);
		int y = (int)(sumy / numy);
	}
	

	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "size/2-obj+sce time:" << t << "s" << endl;
		cout << x << " , " << y << endl;
		t = (double)cv::getTickCount();
	}

	//p = Point((int)x, (int)y);
	p = Point(x, y);
	//-- Draw matches

	//Mat img_matches;
	//drawMatches(img_object, keypoints_object, target, keypoints_scene, good_matches, img_matches, Scalar::all(-1),Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

}

int fun(Point p)
{
	return (int)sqrt(p.x*p.x + p.y*p.y);
}

void GenshinImpact_AutoMap_Matchs::testSURF2()
{
	Mat img_scene(target);
	Mat img_object(object);
	
	static Point hisP[3] = { Point(0,0), Point(0,0), Point(0,0)};
	bool isContinuity = false;

	if ((fun(hisP[1] - hisP[0]) + fun(hisP[2] - hisP[1]))<2000)
	{
		if (hisP[2] .x>150&& hisP[2].x< target.cols-150&& hisP[2].y>150 && hisP[2].y < target.rows - 150)
		{
			isContinuity = true;
			if (isContinuity)
			{
				Mat someMap(target(Rect(hisP[2].x-150, hisP[2].y-150,300,300)));
				detectorTmp = cv::xfeatures2d::SURF::create(minHessian);
				detectorTmp->detectAndCompute(someMap, noArray(), keypoints_sceneTmp, descriptors_sceneTmp);
				detectorTmp->detectAndCompute(img_object, noArray(), keypoints_object, descriptors_object);
				Ptr<DescriptorMatcher> matcherTmp = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
				std::vector< std::vector<DMatch> > knn_matchesTmp;
				std::vector<DMatch> good_matchesTmp;
				matcherTmp->knnMatch(descriptors_object, descriptors_sceneTmp, knn_matchesTmp, 2);
				std::vector<double> lisx;
				std::vector<double> lisy;
				double sumx = 0;
				double sumy = 0;
				for (size_t i = 0; i < knn_matchesTmp.size(); i++)
				{
					if (knn_matchesTmp[i][0].distance < ratio_thresh * knn_matchesTmp[i][1].distance)
					{
						good_matchesTmp.push_back(knn_matchesTmp[i][0]);
						lisx.push_back(((img_object.cols / 2 - keypoints_object[knn_matchesTmp[i][0].queryIdx].pt.x)*1.3 + keypoints_sceneTmp[knn_matchesTmp[i][0].trainIdx].pt.x));
						lisy.push_back(((img_object.rows / 2 - keypoints_object[knn_matchesTmp[i][0].queryIdx].pt.y)*1.3 + keypoints_sceneTmp[knn_matchesTmp[i][0].trainIdx].pt.y));
						sumx += lisx.back();
						sumy += lisy.back();
					}
				}
				Mat img_matches;
				drawMatches(img_object, keypoints_object, someMap, keypoints_sceneTmp, good_matchesTmp, img_matches, Scalar::all(-1),Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

				if (min(lisx.size(), lisy.size()) < 10)
				{
					isContinuity = false;
				}

				double meanx = sumx / lisx.size(); //均值
				double meany = sumy / lisy.size(); //均值
				int x = (int)meanx;
				int y = (int)meany;
				p = Point(x+ hisP[2].x - 150, y+ hisP[2].y - 150);
				
			}
		}
	}
	if(!isContinuity)
	{
		detector->detectAndCompute(img_object, noArray(), keypoints_object, descriptors_object);
		Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
		std::vector< std::vector<DMatch> > knn_matches;
		matcher->knnMatch(descriptors_object, descriptors_scene, knn_matches, 2);

		if (isCout)
		{
			t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
			cout << "matcher->knnMatch time:" << t << "s" << endl;
			t = (double)cv::getTickCount();
		}

		//-- Filter matches using the Lowe's ratio test
		//const float ratio_thresh = 0.8f;
		//std::vector<DMatch> good_matches;

		std::vector<double> lisx;
		std::vector<double> lisy;
		double sumx = 0;
		double sumy = 0;
		for (size_t i = 0; i < knn_matches.size(); i++)
		{
			if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
			{
				//good_matches.push_back(knn_matches[i][0]);
				lisx.push_back(((img_object.cols / 2 - keypoints_object[knn_matches[i][0].queryIdx].pt.x)*1.3 + keypoints_scene[knn_matches[i][0].trainIdx].pt.x));
				lisy.push_back(((img_object.rows / 2 - keypoints_object[knn_matches[i][0].queryIdx].pt.y)*1.3 + keypoints_scene[knn_matches[i][0].trainIdx].pt.y));
				sumx += lisx.back();
				sumy += lisy.back();
			}
		}

		if (min(lisx.size(), lisy.size()) == 0)
		{
			cout << "SURF Match Fail" << endl;
			return;
		}
		cout << "SURF Match Point Number: " << lisx.size() << "," << lisy.size() << endl;
		if (isCout)
		{
			t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
			cout << "good_matches.push_back time:" << t << "s" << endl;
			t = (double)cv::getTickCount();
		}

		double meanx = sumx / lisx.size(); //均值
		double meany = sumy / lisy.size(); //均值
		int x = (int)meanx;
		int y = (int)meany;
		if (min(lisx.size(), lisy.size()) > 15)
		{
			double accumx = 0.0;
			double accumy = 0.0;
			for (int i = 0; i < min(lisx.size(), lisy.size()); i++)
			{
				accumx += (lisx[i] - meanx)*(lisx[i] - meanx);
				accumy += (lisy[i] - meany)*(lisy[i] - meany);
			}

			double stdevx = sqrt(accumx / (lisx.size() - 1)); //标准差
			double stdevy = sqrt(accumy / (lisy.size() - 1)); //标准差

			sumx = 0;
			sumy = 0;
			int numx = 0;
			int numy = 0;
			for (int i = 0; i < min(lisx.size(), lisy.size()); i++)
			{
				if (abs(lisx[i] - meanx) < 3 * stdevx)
				{
					sumx += lisx[i];
					numx++;
				}
				if (abs(lisy[i] - meany) < 3 * stdevy)
				{
					sumy += lisy[i];
					numy++;
				}
			}
			int x = (int)(sumx / numx);
			int y = (int)(sumy / numy);
			p = Point(x, y);
		}
		else
		{
			p= Point(x, y);
		}
	}





	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "size/2-obj+sce time:" << t << "s" << endl;
		cout <<p.x << " , " << p.y << endl;
		t = (double)cv::getTickCount();
	}

	//p = Point((int)x, (int)y);


	hisP[0] = hisP[1];
	hisP[1] = hisP[2];
	hisP[2] = p;
}

void GenshinImpact_AutoMap_Matchs::test()
{
	if (!isInit) return;
	isCanGet = false;
	testSURF2();
	isCanGet = true;
	//testORB();
}

void GenshinImpact_AutoMap_Matchs::test2()
{
	//isCanGet = false;
	Mat img_scene = object(Rect(36,36, object.cols-72,object.rows-72)); //minMap
	static Mat img_object_mask = imread("./Res/ST_mask.bmp", IMREAD_UNCHANGED); //Star
	static Mat tmp;

	cv::matchTemplate(img_object_mask, img_scene, tmp, cv::TM_CCOEFF_NORMED);

	double minVal, maxVal;
	cv::Point minLoc, maxLoc;
	//寻找最佳匹配位置
	cv::minMaxLoc(tmp, &minVal, &maxVal, &minLoc, &maxLoc);

	if (maxVal > 0.75)
	{
		isStarPoint = maxLoc+ Point(img_object_mask.size()) / 2 -Point(img_scene.size())/2;//+Point(11,11)似乎并不需要
		isFindStar = true;
	}
	else
	{
		//isStarPoint = Point(0, 0);
		isFindStar = false;
	}



}

void GenshinImpact_AutoMap_Matchs::test3()
{
	Mat img_scene(object);

	static Mat img_object_mask = imread("./Res/Target_1.bmp", IMREAD_UNCHANGED); //Star
	static Mat tmp;

	cv::matchTemplate(img_object_mask, img_scene, tmp, cv::TM_CCOEFF_NORMED);

	double minVal, maxVal;
	cv::Point minLoc, maxLoc;
	//寻找最佳匹配位置
	cv::minMaxLoc(tmp, &minVal, &maxVal, &minLoc, &maxLoc);

	//cout << minVal << ":" << maxVal << endl;
	rectangle(img_scene, Rect(maxLoc.x , maxLoc.y, img_object_mask.cols, img_object_mask.rows), Scalar(255, 0, 0));
	putText(img_scene, to_string((int)(maxVal * 100)), Point(maxLoc.x, maxLoc.y), 1, 1, Scalar(255, 255, 0));
	//namedWindow("View0", 256);
	//imshow("View0", img_scene);
	//namedWindow("View", 256);
	//imshow("View", tmp);
	//waitKey(1);
	//Mat img_scene = object(Rect(36, 36, object.cols - 72, object.rows - 72)); //minMap
	//static Mat img_object_mask = imread("./Res/ST_mask.bmp", IMREAD_UNCHANGED); //Star
	//static Mat tmp;

	//cv::matchTemplate(img_object_mask, img_scene, tmp, cv::TM_CCOEFF_NORMED);

	//double minVal, maxVal;
	//cv::Point minLoc, maxLoc;
	////寻找最佳匹配位置
	//cv::minMaxLoc(tmp, &minVal, &maxVal, &minLoc, &maxLoc);
	
	//if (maxVal > 0.75)
	//{
	//	isStarPoint = maxLoc + Point(img_object_mask.size()/2) - Point(img_scene.size() / 2);
	//	isFindStar = true;
	//}
	//else
	//{
	//	isFindStar = false;
	//}
}

void GenshinImpact_AutoMap_Matchs::getUID()
{
	isGetUID = false;
	Mat uidType = rectUID(Rect(0,0,60,rectUID.cols));


}

void GenshinImpact_AutoMap_Matchs::testORB()
{
	//ORB 不太行，匹配不上
	Mat tem = object;
	//flip(tem, tem, 0);
	//resize(tem, tem, Size(), 1.5, 1.5);
	Mat test = target;

	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "ORB Start time:" << t << "s" << endl;
		t = (double)cv::getTickCount();
	}


	//FAST_SCORE

	orb->detectAndCompute(tem, Mat(), keyPoints_tem, descriptors_tem, false);
	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "ORB detectAndCompute time:" << t << "s" << endl;
		t = (double)cv::getTickCount();
	}

	//特征匹配是通过使用合适的相似度度量比较特征描述子来执行的。
	//定义特征描述子匹配器
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::MatcherType::BRUTEFORCE);
	//参数MatcherType：匹配器类型，这里使用MatcherType::BRUTEFORCE（暴力匹配算法）

	vector<DMatch> matches;
	//通过描述子匹配器，对两幅图像的描述子进行匹配，也就是将两幅图像中的对应特征点进行匹配；输出的是一个DMatch结构体向量，其每一个DMatch结构体包含一组对应特征点的信息。
	matcher->match(descriptors_tem, descriptors_test, matches, Mat());


	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "ORB BRUTEFORCE time:" << t << "s" << endl;
		t = (double)cv::getTickCount();
	}

	float maxdist = 0;
	for (int i = 0; i < matches.size(); i++)
	{
		//寻找匹配特征点对中匹配质量最差的点对，也就是匹配距离最远的点对，获取该最大距离值
		maxdist = max(maxdist, matches[i].distance);
	}

	vector<DMatch> good_matches;
	for (int j = 0; j < matches.size(); j++)
	{
		//如果匹配特征点对中，某个点对的匹配距离小于某个阈值（可以是最大距离值乘以一个小于1的系数），则可以认为是高度匹配的特征点对
		if (matches[j].distance < 0.7 * maxdist)
		{
			good_matches.push_back(matches[j]);
		}
	}

	if (isCout)
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		cout << "ORB End time:" << t << "s" << endl;
		t = (double)cv::getTickCount();
	}

	//将两幅图像之间的高度匹配的对应特征点使用连线绘制出来，输出一幅将两幅图像拼接起来再进行连线的图像
	//Scalar::all(-1)是选择随机颜色
	Mat result;
	//drawMatches(tem, keyPoints_tem, test, keyPoints_test, good_matches, result, Scalar::all(-1), Scalar::all(-1));
	drawMatches(tem, keyPoints_tem, test, keyPoints_test, good_matches, result, Scalar::all(-1),
		Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	namedWindow("1", WINDOW_FREERATIO);
	imshow("1", result);
}

bool GenshinImpact_AutoMap_Matchs::keySave()
{
	//descriptors_scene keypoints_scene
	return false;
}

bool GenshinImpact_AutoMap_Matchs::keyLoad()
{
	return false;
}

void GenshinImpact_AutoMap_Matchs::setCout(bool _isCout)
{
	isCout = _isCout;
}

bool GenshinImpact_AutoMap_Matchs::getIsCanGet()
{
	return isCanGet;
}

bool GenshinImpact_AutoMap_Matchs::getIsFindStar()
{
	return isFindStar;
}

Point GenshinImpact_AutoMap_Matchs::getFindStar()
{
	return isStarPoint;
}

void GenshinImpact_AutoMap_Matchs::getObjectKeyPoints()
{

}

void GenshinImpact_AutoMap_Matchs::getTargetKeyPoints()
{
	if (!isTargetExist)return;
	switch (mode)
	{
	case 0:
	{
		detector = cv::xfeatures2d::SURF::create(minHessian);
		//detector->detectAndCompute(target, noArray(), keypoints_target, descriptors_target);

		break;
	}
	default:
		break;
	}
}
