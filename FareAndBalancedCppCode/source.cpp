#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <time.h>
#include <Windows.h>
using namespace std;

struct EdgeInfo
{
	int from, to;
	int weight;
	EdgeInfo(int from = 0, int to = 0, int weight = 0) :
		from(from), to(to), weight(weight) {}
};

struct Result
{
	int roadIndex;
	int fare;
	Result(int roadIndex = 0, int fare = 0) :
		roadIndex(roadIndex), fare(fare) {}
};

// 保存单个Case全部数据
struct GraphData
{
	EdgeInfo * roadInfo;
	int R;
	int N;
};

struct ResultItem
{
	bool hasSolution;
	int indexOfCase;
	int count;
	int maxFare;
	vector<Result> roadAndFare;
};

// 记录每个节点的入度
int * countOfIndegree;

// 每个节点到起点/终点的最大距离
int * maxDistanceToStart;
int * maxDistanceToEnd;

// 每个节点到起点/终点分别有多少条距离不同的路线
int * differenceFromStart;
int * differenceFromEnd;

// 按顺序记录遍历过的节点
vector<int> nodesByOrder;

// 存储全部边的信息 <x, y, c>
vector<EdgeInfo> edges;

// 存储节点后所有边的信息 i:[<i, y1, c1>, <i, y2, c2>...]
vector<EdgeInfo> * edgesByNode;

// 存储全部结果
vector<ResultItem> resultsAll;

// 得到单个Case的数据并求解输出
void findAnswer(GraphData *g, int caseIndex)
{
	// 一系列初始化
	ResultItem resultItem;
	resultItem.indexOfCase = caseIndex;

	for (int i = 0; i < g->N + 1; i++)
	{
		edgesByNode[i].clear();
		countOfIndegree[i] = 0;
	}

	edges.clear();

	// 统计各节点入度信息以及按节点查找后继边信息
	for (int i = 0; i < g->R + 1; i++)
	{
		int x = g->roadInfo[i].from;
		int y = g->roadInfo[i].to;
		int c = g->roadInfo[i].weight;

		edgesByNode[x].push_back(EdgeInfo(x, y, c));
		edges.push_back(EdgeInfo(x, y, c));
		countOfIndegree[y] ++;
	}

	for (int i = 0; i < g->N + 1; i++)
	{
		differenceFromStart[i] = 0;
		differenceFromEnd[i] = 0;
		maxDistanceToStart[i] = 0;
		maxDistanceToEnd[i] = 0;
	}

	nodesByOrder.clear();
	queue<int> nodesToScan;

	// 寻找入度为 0的节点入队
	// 也就是该步骤目的是找到起始节点
	for (int i = 1; i < g->N; i++)
	{
		if (countOfIndegree[i] == 0)
		{
			nodesToScan.push(i);
		}
	}
	// 从起始节点开始
	while (!nodesToScan.empty())
	{
		int nodeIndexTemp = nodesToScan.front();
		nodesToScan.pop();
		nodesByOrder.push_back(nodeIndexTemp);

		// 不断更新并记录每个节点到起点的最大距离
		// 同时存储每个节点到起点有多少条距离不同的路线
		for (int i = 0; i < edgesByNode[nodeIndexTemp].size(); i++)
		{
			EdgeInfo edgeTemp = edgesByNode[nodeIndexTemp][i];
			if ((maxDistanceToStart[edgeTemp.to] != 0)
				&& (maxDistanceToStart[edgeTemp.to] !=
					(maxDistanceToStart[edgeTemp.from] + edgeTemp.weight)))
			{
				differenceFromStart[edgeTemp.to] += 1;
			}

			differenceFromStart[edgeTemp.to] += differenceFromStart[edgeTemp.from];

			maxDistanceToStart[edgeTemp.to] = max(maxDistanceToStart[edgeTemp.to]
				, maxDistanceToStart[edgeTemp.from] + edgeTemp.weight);

			countOfIndegree[edgeTemp.to] -= 1;
			// 若入度为0则进队，进入下一次循环
			if (countOfIndegree[edgeTemp.to] == 0)
			{
				nodesToScan.push(edgeTemp.to);
			}
		}
	}

	// 计算每个节点到终点的最远距离
	// 同时记录每个节点到终点距离不同的路线数量
	for (int i = nodesByOrder.size() - 1; i >= 0; i--)
	{
		int nodeTemp = nodesByOrder[i];
		if (nodeTemp == g->N)
		{
			// 终点节点略过
			continue;
		}
		for (EdgeInfo edgeTemp : edgesByNode[nodeTemp])
		{
			maxDistanceToEnd[edgeTemp.from] = max(maxDistanceToEnd[edgeTemp.from]
				, maxDistanceToEnd[edgeTemp.to] + edgeTemp.weight);
			differenceFromEnd[edgeTemp.from] += differenceFromEnd[edgeTemp.to];
		}
		if (edgesByNode[nodeTemp].size() != 0)
		{
			EdgeInfo edgeTemp = edgesByNode[nodeTemp][0];
			int distanceTemp = maxDistanceToEnd[edgeTemp.to] + edgeTemp.weight;

			// 下面部分是得到该节点右侧有多少个路线距离不一样
			for (int j = 0; j < edgesByNode[nodeTemp].size(); j++)
			{
				if (maxDistanceToEnd[edgesByNode[nodeTemp][j].to] + edgesByNode[nodeTemp][j].weight != distanceTemp)
				{
					differenceFromEnd[nodeTemp] += 1;
				}
			}
		}
	}

	// 到这里已经确定了每个节点的左侧有多少不相同的路，以及其右侧
	for (int i = 1; i <= g->N; i++)
	{
		// 这种情况下是无解的，这样的话节点i [1~N]需要同时修改其左侧和右侧，不满足题意
		if (differenceFromStart[i] > 0 && differenceFromEnd[i] > 0)
		{
			resultItem.hasSolution = false;
			resultsAll.push_back(resultItem);
			return;
		}
	}

	vector<Result> results;

	for (int i = 0; i < g->R; i++)
	{
		EdgeInfo edgeTemp = edges[i];

		// 如果某条边的起点到图的起点没有距离不同的路径，切该边的终点到图的终点也没有距离不同的路径
		// 则该边的起点和终点之间可能存在需要修改的边
		if (differenceFromStart[edgeTemp.from] == 0 && differenceFromEnd[edgeTemp.to] == 0 &&
			(differenceFromStart[edgeTemp.to] > 0 || differenceFromEnd[edgeTemp.from] > 0))
		{
			// 需要增加的收费等于 H - x到start最大值 - y到end最大值 - <x, y>本身的权重值
			// 可以理解为把这段权重加上多少的收费才能在不修改其左右的情况下达到数值 H
			int fare = maxDistanceToStart[g->N] -
				(maxDistanceToStart[edgeTemp.from] + maxDistanceToEnd[edgeTemp.to] + edgeTemp.weight);
			if (fare > 0)
			{
				// 保存结果
				results.push_back(Result(i + 1, fare));
			}
		}
	}

	resultItem.hasSolution = true;
	resultItem.indexOfCase = caseIndex;
	resultItem.count = results.size();
	resultItem.maxFare = maxDistanceToStart[g->N];
	for (int i = 0; i < results.size(); i++)
	{
		resultItem.roadAndFare = results;
	}
	resultsAll.push_back(resultItem);
}

// 使用动态数组初始化一些变量
void initVariables(int N, int R)
{
	// 每个节点的入度
	delete[] countOfIndegree;
	countOfIndegree = NULL;
	countOfIndegree = new int[N + 1];

	// 每个节点到起点的最大距离
	delete[] maxDistanceToStart;
	maxDistanceToStart = NULL;
	maxDistanceToStart = new int[N + 1];

	// 每个节点到终点的最大距离
	delete[] maxDistanceToEnd;
	maxDistanceToEnd = NULL;
	maxDistanceToEnd = new int[N + 1];

	// 每个节点到起点不同距离的路径数量
	delete[] differenceFromStart;
	differenceFromStart = NULL;
	differenceFromStart = new int[N + 1];

	// 每个节点到终点不同距离的路径数量
	delete[] differenceFromEnd;
	differenceFromEnd = NULL;
	differenceFromEnd = new int[N + 1];

	// 按照节点值查找其全部后继边
	edgesByNode->clear();
	delete[] edgesByNode;
	edgesByNode = NULL;
	edgesByNode = new vector<EdgeInfo>[N + 1];
}

// 打印当前图全部数据
void printCaseData(GraphData *g)
{
	cout << g->N << " " << g->R << endl;
	for (int i = 0; i < g->R; i++)
	{
		cout << g->roadInfo[i].from << " " << g->roadInfo[i].to << " " << g->roadInfo[i].weight << endl;
	}
}

// 释放内存
void clearMemory(GraphData *g)
{
	// 释放该case内存
	delete[] countOfIndegree;
	countOfIndegree = NULL;
	delete[] maxDistanceToEnd;
	maxDistanceToEnd = NULL;
	delete[] maxDistanceToStart;
	maxDistanceToStart = NULL;
	delete[] differenceFromEnd;
	differenceFromEnd = NULL;
	delete[] differenceFromStart;
	differenceFromStart = NULL;
	delete[] edgesByNode;
	edgesByNode = NULL;
	delete[] g->roadInfo;
	g->roadInfo = NULL;
}

// 从文件中读取并计算
void readFileAndEval(string filePath)
{
	int indexStart = 0;
	int indexEnd = 0;
	int indexNow = 0;
	int indexInCase = 0;
	int caseIndex = 0;

	// 给定初始值，避免报错
	edgesByNode = new vector<EdgeInfo>[1];
	countOfIndegree = new int[1];
	maxDistanceToStart = new int[1];
	maxDistanceToEnd = new int[1];
	differenceFromStart = new int[1];
	differenceFromEnd = new int[1];

	GraphData graphData;
	graphData.roadInfo = new EdgeInfo[1];

	ifstream inFile(filePath);
	if (!inFile.is_open())
	{
		cout << "fail to read this file: " << filePath << endl;
		clearMemory(&graphData);
		return;
	}

	while (true)
	{
		if (indexNow == indexStart)
		{
			// 获取单个Case的N和R值
			int N, R;
			inFile >> N;
			if (N == 0)
			{
				clearMemory(&graphData);
				break;
			}
			inFile >> R;

			graphData.N = N;
			graphData.R = R;

			delete[] graphData.roadInfo;
			graphData.roadInfo = NULL;
			graphData.roadInfo = new EdgeInfo[R + 1];

			initVariables(N, R);

			// 计算本次Case读取终点下标
			indexNow += 2;
			indexEnd = indexStart + 2 + R * 3;
		}
		if (indexNow <= indexEnd)
		{
			int x, y, c;
			inFile >> x;
			inFile >> y;
			inFile >> c;

			graphData.roadInfo[indexInCase].from = x;
			graphData.roadInfo[indexInCase].to = y;
			graphData.roadInfo[indexInCase].weight = c;
			indexInCase++;
			indexNow += 3;

			if (indexNow == indexEnd)
			{
				indexStart = indexEnd;
				indexInCase = 0;
				// 读取数据可视化
				// printCaseData(&graphData);
				findAnswer(&graphData, ++caseIndex);
			}
		}
	}
	inFile.close();
}

// 打印全部计算结果
void printAllResults()
{
	for (int i = 0; i < resultsAll.size(); i++)
	{
		if (resultsAll[i].hasSolution == false)
		{
			cout << "Case " << resultsAll[i].indexOfCase << ": No solution" << endl;
			continue;
		}
		cout << "Case " << resultsAll[i].indexOfCase << ": " << resultsAll[i].count << " " << resultsAll[i].maxFare << endl;
		for (int j = 0; j < resultsAll[i].roadAndFare.size(); j++)
		{
			cout << resultsAll[i].roadAndFare[j].roadIndex << " " << resultsAll[i].roadAndFare[j].fare << endl;
		}
	}
}

int main()
{
	string pathReal = "data/fare.txt";
	readFileAndEval(pathReal);
	printAllResults();
	return 0;
}