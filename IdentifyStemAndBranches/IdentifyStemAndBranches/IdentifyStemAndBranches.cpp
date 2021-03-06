// IdentifyStemAndBranches.cpp: First identifies stem, then branches using longest path algorithm

#define USE_MATH_DEFINES

#include "stdafx.h"
#include <plyall.h>
#include <ply.h>

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <math.h>
#include <algorithm>
#include <fstream>
#include <ostream>
#include <algorithm>
#include <queue>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <cmath>

using namespace std;

struct internal {
	struct Vertex { float x, y, z; };
	struct Edge { int v1, v2; };
	struct Face {
		unsigned char nvts;
		int verts[3];
		float s;
	};
};
struct VertexWithMsure
{
	float width, radius;
	float x, y, z;
	float compType;
	int inDiameter;

};

void fillComponentIterative(std::map<int, int> &visitedMap, int i, std::map<int, std::vector<int> > vertEdgeMapping, std::vector<internal::Edge> edges, std::vector<int> vertComps, std::vector<int> &newEdgeComp, std::vector<int> &newVertComp, float lowerRadiiThresh, vector<VertexWithMsure> vts) {

	//BFS to find high radius component of edges and vertices
	queue<int> vQ;
	vQ.push(i);
	while (!vQ.empty()) {
		int node = vQ.front();
		vQ.pop();
		if (visitedMap.find(node) == visitedMap.end()) {
			visitedMap[node] = true;
			newVertComp.push_back(node);
			for (int j = 0; j < vertEdgeMapping[node].size(); j++) {
				
				int eIndex = vertEdgeMapping[node][j];
				//Choose endpoint of edge which hasn't been visited yet
				if (visitedMap.find(edges[eIndex].v1) == visitedMap.end()) {
					//Stop visiting edges when radius decreases below threshold
					if (vts[edges[eIndex].v1].radius > lowerRadiiThresh) {
						newEdgeComp.push_back(eIndex);

						vQ.push(edges[eIndex].v1);
					}
				}

				if (visitedMap.find(edges[eIndex].v2) == visitedMap.end()) {
					//Stop visiting edges when radius decreases below threshold
					if (vts[edges[eIndex].v2].radius > lowerRadiiThresh) {
						newEdgeComp.push_back(eIndex);
						vQ.push(edges[eIndex].v2);
					}
				}
			}
		}
	}

}

float euclideanDistance(VertexWithMsure v1, VertexWithMsure v2) {
	return sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y) + (v1.z - v2.z)*(v1.z - v2.z));
}

float getLengthOfComponent(std::vector<int> compEdges, std::vector<internal::Edge> edges, std::vector<VertexWithMsure> vertices) {
	cout << compEdges.size() << " " << edges.size() << " " << vertices.size() << endl;
	float dist = 0.0;

	for (int i = 0; i < compEdges.size(); i++) {
		dist += euclideanDistance(vertices[edges[compEdges[i]].v1], vertices[edges[compEdges[i]].v2]);
	}
	return dist;
}

float gaussianFactor(float val, float factor) {
	return exp(-(val*val) / 2.0*factor*factor);
}

typedef std::pair<int, int> E;

float getRangeScore(vector< vector<E> >& mstVertexEdges, int seedVertex, vector<VertexWithMsure>& allVertices, map<int, bool>& vertexInDiameter, int range) {
	int currentSeed = seedVertex;
	int ct = 0;
	float avgRadii = 0.0;
	map<int, bool> inRangeAlready;
	queue<int> currentSeeds; currentSeeds.push(currentSeed);
	vector<int> nextSeeds;
	while (!currentSeeds.empty() && ct < range) {

		currentSeed = currentSeeds.front();
		ct += 1;
		currentSeeds.pop();
		avgRadii += allVertices[currentSeed].radius;

		vector<E> vertEdges = mstVertexEdges[currentSeed];
		E currentEdge;
		for (int j = 0; j < vertEdges.size(); j++) {
			currentEdge = vertEdges[j];
			int nextSeed;
			if (currentEdge.first == currentSeed) {
				nextSeed = currentEdge.second;

			}
			else {
				nextSeed = currentEdge.first;
			}
			if (inRangeAlready.find(nextSeed) == inRangeAlready.end()) {

				if (vertexInDiameter.find(nextSeed) == vertexInDiameter.end()) {

					nextSeeds.push_back(nextSeed);
				}
			}
		}
	}
	for (int l = 0; l < nextSeeds.size(); l++) {
		currentSeeds.push(nextSeeds[l]);
	}

	inRangeAlready.clear();
	return avgRadii / (float)ct;
}

//Vector dot product
double dotProd(vector<float> v1, vector<float> v2) {
	double val = (double)(v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]);
	return val;
}

vector<float> vecDiff(VertexWithMsure v1, VertexWithMsure v2) {
	return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

float getMagnitude(vector<float> vec) {
	float sum = 0.0;
	for (int i = 0; i < vec.size(); i++) {
		sum += (vec[i] * vec[i]);
	}
	return sqrtf(sum);
}

//Find angle between vectors using dot product
float angleBetweenVector(vector<float> v1, vector<float> v2) {
	return std::acos((double)dotProd(v1, v2) / (getMagnitude(v1)*getMagnitude(v2)));
}

vector<float> crossProduct(vector<float> v1, vector<float> v2) {
	vector<float> cross;
	cross.push_back(v1[1] * v2[2] - v1[2] * v2[1]);
	cross.push_back(v1[2] * v2[0] - v1[0] * v2[2]);
	cross.push_back(v1[0] * v2[1] - v1[1] * v2[0]);

	return cross;

}

void normalize(vector<float>& vec) {
	float mag = getMagnitude(vec);
	for (int i = 0; i < vec.size(); i++) {
		vec[i] /= mag;
	}
}

vector<vector<int> > buildBranchesFromVertexForwardBFS(int& vertexSeed, vector<bool>& inDiameter, std::vector<internal::Edge>& edges, vector< std::vector<int> >& vertIndexEdgeIndices, vector<VertexWithMsure>& stemVertices, vector<int>& visited, int& branchSizeMin, int& curvatureWindow, int& maxBranchLen,
	vector<float>& stemCentroid, vector<int>& loop, float& radiusTolerance, float& emergenceAngleThresh, float& tipAngleThresh, float& tortuosityThresh, vector<float>& stemVec, vector<float>& emergenceAngles, vector<float>& tipAngles, vector<float>& branchLengths, vector<float>& tipLengths, float& emergenceLowerRadius, int& emergeWindow) {

	vector<vector<int> > finalBranchEdges;
	//Find possible seed points first
	queue<int> seedPtQ;
	seedPtQ.push(vertexSeed);
	vector<int> junctionsWithinRadius;
	vector<bool> thisBranchVisitedWithinRadius(stemVertices.size(), false);
	while (!seedPtQ.empty()) {
		int currentVertex = seedPtQ.front();
		seedPtQ.pop();
		if (thisBranchVisitedWithinRadius[currentVertex] == false) {
			if (euclideanDistance(stemVertices[currentVertex], stemVertices[vertexSeed]) < radiusTolerance*stemVertices[vertexSeed].radius) {
				if (vertIndexEdgeIndices[currentVertex].size() > 2 || currentVertex == vertexSeed) {
					junctionsWithinRadius.push_back(currentVertex);
				}
			}
			thisBranchVisitedWithinRadius[currentVertex] = true;
			vector<int> neighbors = vertIndexEdgeIndices[currentVertex];
			for (int i = 0; i < neighbors.size(); i++) {
				internal::Edge neighborEdge = edges[neighbors[i]];
				int neighborVertexIndex = -1;
				if (neighborEdge.v1 == currentVertex) {
					//Means neighbor is v2
					neighborVertexIndex = neighborEdge.v2;
				}
				else {
					if (neighborEdge.v2 == currentVertex) {
						//Means neighbor is v1
						neighborVertexIndex = neighborEdge.v1;
					}
				}
				if (neighborVertexIndex != -1) {
					if (inDiameter[neighborVertexIndex] == false) {
						if (thisBranchVisitedWithinRadius[neighborVertexIndex] == false) {
							if (euclideanDistance(stemVertices[neighborVertexIndex], stemVertices[vertexSeed]) < radiusTolerance*stemVertices[vertexSeed].radius) {
								seedPtQ.push(neighborVertexIndex);
							}
						}
					}
				}
			}
		}

	}

	struct node
	{
		int index;
		int dist;

		node(int i1, int i2) : index(i1), dist(i2)
		{
		}

		bool operator<(const struct node other) const
		{
			return dist < other.dist;
		}
	};

	for (int i = 0; i < junctionsWithinRadius.size(); i++) {
		int seed = junctionsWithinRadius[i];
		vector<int> prevNode(stemVertices.size(), -1);
		prevNode[seed] = seed;
		vector<int> prevEdge(stemVertices.size(), -1);
		vector<int> pathLen(stemVertices.size(), -1);
		vector<int> thisBranchVisited(stemVertices.size(), false);
		pathLen[seed] = 0;
		vector<int> candidateEndPts;
		queue<int> branchVertexQueue;
		branchVertexQueue.push(seed);
		priority_queue<node> branchLenQ;
		vector<float> pathDist(stemVertices.size(), -1);
		pathDist[seed] = 0.0;
		while (!branchVertexQueue.empty()) {
			int currentVertex = branchVertexQueue.front();
			branchVertexQueue.pop();
			if (inDiameter[currentVertex] == false) {
				if (thisBranchVisited[currentVertex] == false) {
					if (visited[currentVertex] == -1 || vertIndexEdgeIndices[currentVertex].size() > 2) {
						thisBranchVisited[currentVertex] = true;
						vector<int> neighbors = vertIndexEdgeIndices[currentVertex];

						if (pathLen[currentVertex] > branchSizeMin && pathLen[currentVertex] < maxBranchLen) {
							branchLenQ.push(node(currentVertex, pathLen[currentVertex]));
						}
						for (int j = 0; j < neighbors.size(); j++) {
							internal::Edge neighborEdge = edges[neighbors[j]];
							int neighborVertexIndex = -1;
							if (neighborEdge.v1 == currentVertex) {
								//Means neighbor is v2
								neighborVertexIndex = neighborEdge.v2;
							}
							else {
								if (neighborEdge.v2 == currentVertex) {
									//Means neighbor is v1
									neighborVertexIndex = neighborEdge.v1;
								}
							}
							if (neighborVertexIndex != -1) {
								if (inDiameter[neighborVertexIndex] == false) {
									if (thisBranchVisited[neighborVertexIndex] == false) {
										//Need to add constraint for distance to stem?
										if (visited[neighborVertexIndex] == -1 || vertIndexEdgeIndices[neighborVertexIndex].size() > 2) {
											if (pathLen[currentVertex] < maxBranchLen) {
												pathLen[neighborVertexIndex] = pathLen[currentVertex] + 1;
												prevNode[neighborVertexIndex] = currentVertex;
												prevEdge[neighborVertexIndex] = neighbors[j];
												pathDist[neighborVertexIndex] = pathDist[currentVertex] + euclideanDistance(stemVertices[neighborVertexIndex], stemVertices[currentVertex]);
												branchVertexQueue.push(neighborVertexIndex);

												if (visited[neighborVertexIndex] == 1 && vertIndexEdgeIndices[neighborVertexIndex].size() > 2) {
													cout << "Encountered intersection with other branch " << endl;
												}
											}
										}
									}
								}
							}
						}
					}

				}
			}
		}
		if (branchLenQ.size() > 0) {
			int possibleEndPts = max(1, (int)branchLenQ.size() / 10);
			vector<bool> badEndPts(stemVertices.size(), false);
			vector < vector<int> > pathMapping;
			for (int k = 0; k < stemVertices.size(); k++) {
				pathMapping.push_back({});
			}

			vector<int> bestBranch = {};
			int bestEndPt = -1;
			float bestScore = 0.0;
			for (int j = 0; j < possibleEndPts; j++) {
				node next = branchLenQ.top();
				branchLenQ.pop();
				if (badEndPts[next.index] == false) {
					vector<int> pathVts;
					if (pathMapping[next.index].size() > 0) {
						pathVts = pathMapping[next.index];
					}
					else {
						int currentPathPt = next.index;
						int breakIdx = 0;
						while (currentPathPt != vertexSeed && prevNode[currentPathPt] != currentPathPt && currentPathPt != junctionsWithinRadius[i] && prevNode[currentPathPt] != junctionsWithinRadius[i]) {
							pathVts.push_back(currentPathPt);
							currentPathPt = prevNode[currentPathPt];
							breakIdx++;
						}
						std::reverse(pathVts.begin(), pathVts.end());
						for (int k = 0; k < pathVts.size(); k++) {
							vector<int> sub(&pathVts[0], &pathVts[k]);
							pathMapping[pathVts[k]] = sub;
						}
					}

					float pathScore = 0.0;
					int junctionCt = 0;
					for (int k = 0; k < pathVts.size(); k++) {

						if (vertIndexEdgeIndices[pathVts[k]].size() > 2) {
							junctionCt += 1;
							int windowBack = min(curvatureWindow, k);
							int windowAhead = min(curvatureWindow, (int)pathVts.size() - k - 1);
							float xDiff = 0.0;
							float yDiff = 0.0;
							float zDiff = 0.0;
							float windowDistance = 0.0;
							float secondDerivAvg = 0.0;
							for (int k1 = k - windowBack; k1 < k + windowAhead - 2; k1++) {
								float xDiff2 = (stemVertices[pathVts[k1 + 2]].x - stemVertices[pathVts[k1 + 1]].x) - (stemVertices[pathVts[k1 + 1]].x - stemVertices[pathVts[k1]].x);
								float yDiff2 = (stemVertices[pathVts[k1 + 2]].y - stemVertices[pathVts[k1 + 1]].y) - (stemVertices[pathVts[k1 + 1]].y - stemVertices[pathVts[k1]].y);
								float zDiff2 = (stemVertices[pathVts[k1 + 2]].z - stemVertices[pathVts[k1 + 1]].z) - (stemVertices[pathVts[k1 + 1]].z - stemVertices[pathVts[k1]].z);
								secondDerivAvg += getMagnitude({ xDiff2,yDiff2,zDiff2 });
								windowDistance += euclideanDistance(stemVertices[pathVts[k1 + 1]], stemVertices[pathVts[k1]]);
							}
							secondDerivAvg /= windowDistance;
							float curvatureScore = secondDerivAvg;
							curvatureScore = gaussianFactor(curvatureScore, 0.1);
							curvatureScore *= (((pathVts.size() + 1) / pathVts.size()) - ((k + 1) / pathVts.size()));
							pathScore += curvatureScore;
						}
					}

					if (junctionCt == 0) {
						pathScore = pathVts.size();
						if (pathScore > bestScore) {
							bestScore = pathScore;
							bestBranch = pathVts;
							bestEndPt = next.index;
						}
						//path has no junctions
					}
					else {
						pathScore /= (float)junctionCt;
						if (pathScore > bestScore) {
							bestScore = pathScore;
							bestBranch = pathVts;
							bestEndPt = next.index;
						}
					}
					pathVts.clear();
					pathVts.shrink_to_fit();
				}

			}
			if (bestBranch.size() > 0) {
				int currentPathPt = bestEndPt;
				int breakIdx = 0;
				vector<int> pathEdges;
				vector<int> branchVts;

				bool emergenceAngleWithinRange = true;
				float farthestEndPtDist = 0.0;
				float totalDist = 0.0;
				map<int, bool> seen;
				//Only include branch up to portion outside stem
				while (currentPathPt != vertexSeed && prevNode[currentPathPt] != currentPathPt && currentPathPt != seed && euclideanDistance(stemVertices[currentPathPt], stemVertices[vertexSeed]) > stemVertices[vertexSeed].radius) {
					pathEdges.push_back(prevEdge[currentPathPt]);
					branchVts.push_back(currentPathPt);
					if (seen.find(currentPathPt) != seen.end()) {
						cout << " bug, already saw vertex " << currentPathPt << " along path " << endl;
					}
					seen[currentPathPt] = true;
					float dist = euclideanDistance(stemVertices[currentPathPt], stemVertices[vertexSeed]);
					totalDist += euclideanDistance(stemVertices[currentPathPt], stemVertices[prevNode[currentPathPt]]);
					if (dist > farthestEndPtDist) {
						farthestEndPtDist = dist;

					}
					currentPathPt = prevNode[currentPathPt];
					breakIdx++;
				}

				std::reverse(branchVts.begin(), branchVts.end());
				

				vector<int> seedBranchEdges = pathEdges;
				if (seedBranchEdges.size() > branchSizeMin) {
					double * branchVtArr = new double[branchVts.size() * 3];
					for (int o = 0; o < branchVts.size(); o++) {
						branchVtArr[3 * o] = stemVertices[branchVts[o]].x;
						branchVtArr[3 * o + 1] = stemVertices[branchVts[o]].y;
						branchVtArr[3 * o + 2] = stemVertices[branchVts[o]].z;
					}
					/**
					alglib::ae_int_t infoB;

					// scalar values that describe variances along each eigenvector
					alglib::real_1d_array eigValuesB;

					// unit eigenvectors which are the orthogonal basis that we want
					alglib::real_2d_array eigVectorsB;
					alglib::real_2d_array ptInputB;
					ptInputB.setcontent(branchVts.size(), 3, branchVtArr);
					// perform the analysis
					pcabuildbasis(ptInputB, branchVts.size(), 3, infoB, eigValuesB, eigVectorsB);
					// now the vectors can be accessed as follows:
					double basis0_xB = eigVectorsB[0][0];
					double basis0_yB = eigVectorsB[1][0];
					double basis0_zB = eigVectorsB[2][0];

					double basis1_xB = eigVectorsB[0][1];
					double basis1_yB = eigVectorsB[1][1];
					double basis1_zB = eigVectorsB[2][1];

					double basis2_xB = eigVectorsB[0][2];
					double basis2_yB = eigVectorsB[1][2];
					double basis2_zB = eigVectorsB[2][2];
					vector<float> pca = { (float)basis0_xB , (float)basis0_yB, (float)basis0_zB, (float)basis1_xB , (float)basis1_yB,  (float)basis1_zB, (float)basis2_xB , (float)basis2_yB, (float)basis2_zB };
					**/
					VertexWithMsure vt = stemVertices[vertexSeed];
					vector<int> emergenceVtWindow;
					int emergeIndex = min(branchSizeMin - 1, 24);
					for (int k = min(branchSizeMin - 1, 24); k < branchVts.size(); k++) {
						//branch does not emerge until beyond radius of junction, and radius thin
						if (euclideanDistance(stemVertices[branchVts[k]], stemVertices[vertexSeed]) > stemVertices[vertexSeed].radius*radiusTolerance) {
							if (euclideanDistance(stemVertices[branchVts[k]], stemVertices[vertexSeed]) > euclideanDistance(stemVertices[branchVts[emergeIndex]], stemVertices[vertexSeed])) {
								emergeIndex = k;
								if (stemVertices[branchVts[emergeIndex]].radius < emergenceLowerRadius) {

									break;
								}
							}

						}
					}
					
					int emergeEnd = min(emergeIndex + emergeWindow, (int)branchVts.size() - 1);
					vector<float> emergenceVec = { stemVertices[branchVts[emergeEnd]].x - stemVertices[branchVts[emergeIndex]].x, stemVertices[branchVts[emergeEnd]].y - stemVertices[branchVts[emergeIndex]].y, stemVertices[branchVts[emergeEnd]].z - stemVertices[branchVts[emergeIndex]].z };
					normalize(emergenceVec);

					float emergenceAngle = (180.0 / 3.1415926535897932384626433832795) * acos(dotProd(emergenceVec, stemVec) / (getMagnitude(emergenceVec) *  getMagnitude(stemVec)));
					vector<float> tipVector = { stemVertices[branchVts[branchVts.size() - 1]].x - stemVertices[branchVts[0]].x, stemVertices[branchVts.size() - 1].y - stemVertices[branchVts[0]].y, stemVertices[branchVts.size() - 1].z - stemVertices[branchVts[0]].z };
					normalize(tipVector);
					float tipAngle = (180.0 / 3.1415926535897932384626433832795) * acos(dotProd(tipVector, stemVec) / (getMagnitude(tipVector) *  getMagnitude(stemVec)));
					float tortuosity = totalDist / farthestEndPtDist;
					if (emergenceAngle < emergenceAngleThresh && tipAngle < tipAngleThresh && tortuosity < tortuosityThresh) {

						if (seedBranchEdges.size() > branchSizeMin) {

							finalBranchEdges.push_back(seedBranchEdges);
							cout << "Added branch of size " << seedBranchEdges.size() << endl;
							for (int k = 0; k < seedBranchEdges.size(); k++) {
								visited[edges[seedBranchEdges[k]].v1] = 1;
								visited[edges[seedBranchEdges[k]].v2] = 1;
							}
							emergenceAngles.push_back(emergenceAngle);
							tipAngles.push_back(tipAngle);
							branchLengths.push_back(totalDist);
							tipLengths.push_back(farthestEndPtDist);
							//pcas.push_back(pca);
						}
					}
					else {
						cout << "branch not qualified, with emergence angle " << emergenceAngle << " " << emergenceAngleThresh << " tip angle " << tipAngle << " " << tipAngleThresh << " tortuosity " << tortuosity << " " << tortuosityThresh << endl;
					}
					bestBranch.clear();
					bestBranch.shrink_to_fit();
					thisBranchVisited.clear();
					thisBranchVisited.shrink_to_fit();
					pathLen.clear();
					pathLen.shrink_to_fit();
					prevNode.clear();
					prevNode.shrink_to_fit();
					prevEdge.clear();
					prevEdge.shrink_to_fit();
				}
			}
		}
	}
	//Re-calculate angle using whole stem


	return finalBranchEdges;
}

bool sortbysec(const std::tuple<int, float>& a, const std::tuple<int, float>& b)
{
	return (get<1>(a) < get<1>(b));
}


int main(int argc, char** argv)
{
	using namespace boost;

	std::map<std::string, PlyProperty> v_props_map;
	//Output file for branch traits

	string outMeasureFile;
	float upperRadiiThresh = 10.0;
	float lowerRadiiThresh = 1.0;
	float compThreshSize = 200.0;
	int branchSizeMin = 200;
	int curvatureWindow = 30;
	int maxBranchLength = 900;
	float radiusTol = 1.2;
	float emergenceAngleThresh = 360.0;
	float tipAngleThresh = 120.0;
	float tortuosityThresh = 2.5;
	float emergenceLowerRadius = 2.0;
	float lowerStemThresh = 1.0;
	int emergeWindow = 30;
	char * inFile = NULL;
	string outFile;
	for (int i = 0; i < argc-1; i++) {
		if (((string)argv[i]).substr(0, 2) == "--") {
			string arg = (string)argv[i];
			arg.erase(0,2);
			if (arg == "inFile") {
				inFile = argv[i + 1]; //Input skeleton file: don't need in panel since the input skeleton will already be preloaded 
			}
			if (arg == "outFile") {
				outFile = argv[i + 1]; //Output skeleton file: add button to export branches
			}
			if(arg == "outMeasureFile") {
				outMeasureFile = argv[i + 1]; //Output measurements: add button to export traits file
			}
			if (arg == "upperRadius") {
				//Upper radius threshold for identifying seed point during stem identification
				upperRadiiThresh = stof(argv[i + 1]); //Parameter which goes in stem identification portion, default value 10.0 voxels
			}
			if (arg == "lowerRadius") {
				//Lower radius threshold for identifying seed point during stem identification
				lowerRadiiThresh = stof(argv[i + 1]); //Parameter which goes in stem identification portion, default value 1.0 voxels
			}
			if (arg == "componentSize") {
				//Don't need to include in panel! Identifies stem region if the number of vertices exceeds this value
				compThreshSize = stof(argv[i + 1]); //Dont need in panel, use default value of 200 vertices 
			}
			if (arg == "minBranchSize") {
				//Lower threshold for minimum branch size
				branchSizeMin = stoi(argv[i + 1]); //Parameter which goes in branch identification, default value 200 vertices 
			}
			if (arg == "curvatureWindow") {
				//Window size used to calculate curvature
				curvatureWindow = stoi(argv[i + 1]); //Optional parameter, use default value of 30 
			}
			if (arg == "maxBranchLength") {
				//Upper threshold for maximum branch size
				maxBranchLength = stoi(argv[i + 1]); //Parameter which goes in branch identification, default value 900 vertices
			}
			if (arg == "radiusTolerance") {
				//How far from stem to continue to search for junction points, as starting points to begin branch search
				radiusTol = stof(argv[i + 1]); //Parameter which goes in branch identification, default value 1.2
			}
			if (arg == "emergenceAngle") {
				//Upper threshold for emergence angle; not reliable, so use as optional parameter
				emergenceAngleThresh = stof(argv[i + 1]); //Optional parameter, use default value of 360.0 degres
			}
			if (arg == "tipAngle") {
				//Upper threshold for tip angle
				tipAngleThresh = stof(argv[i + 1]); //Parameter which goes in branch identification, default value 120.0 degrees
			}
			if (arg == "tortuosity") {
				//Upper threshold for tortuosity, which equals skeleton distance / straight line euclidean distance from junction point to end of branch
				tortuosityThresh = stof(argv[i + 1]); //Parameter which goes in branch identification, default value 2.5;
			}
			if (arg == "emergenceLowerRadius") {
				//Lower radius threshold to stop window for calculating emergence angle
				emergenceLowerRadius = stof(argv[i + 1]);//Optional parameter, default value of 2.0 voxels
			}
			if (arg == "emergeWindow") {
				//Maximum absolute window size used to calculate the emergence angle
				emergeWindow = stoi(argv[i + 1]);//Optional parameter, default value of 30 vertices
			}
			if (arg == "lowerStemThresh") {
				//Don't need to include in panel! Used to increase radius of stem when the stem has a hole in it.
				lowerStemThresh = stof(argv[i+1]); //Don't need in panel, use default value of 1.0
			}
		}
	}

	PlyProperty v_props[] = {
		{ (char*)"bt2", Float32, Float32, offsetof(VertexWithMsure, width), 0, 0, 0, 0 },
	{ (char*)"radius", Float32, Float32, offsetof(VertexWithMsure, radius), 0, 0, 0, 0 },
	//{ (char*)"bt1", Float32, Float32, offsetof(VertexWithMsure, bt1), 0, 0, 0, 0 },
	{ (char*)"x", Float32, Float32, offsetof(VertexWithMsure, x), 0, 0, 0, 0 },
	{ (char*)"y", Float32, Float32, offsetof(VertexWithMsure, y), 0, 0, 0, 0 },
	{ (char*)"z", Float32, Float32, offsetof(VertexWithMsure, z), 0, 0, 0, 0 }
	};
	v_props_map["bt2"] = v_props[0];
	v_props_map["radius"] = v_props[1];
	v_props_map["x"] = v_props[2];
	v_props_map["y"] = v_props[3];
	v_props_map["z"] = v_props[4];
	std::map<std::string, PlyProperty> e_props_map;
	PlyProperty e_props[] = {
		{ (char*)"vertex1", Int32, Int32, offsetof(internal::Edge, v1), PLY_SCALAR, 0, 0, 0 },
	{ (char*)"vertex2", Int32, Int32, offsetof(internal::Edge, v2), PLY_SCALAR, 0, 0, 0 }
	};
	e_props_map["vertex1"] = e_props[0];
	e_props_map["vertex2"] = e_props[1];
	std::map<std::string, PlyProperty> f_props_map;
	PlyProperty f_props[] = {
		(char*)"vertex_indices", Int32, Int32, offsetof(ply::Face, verts),
		PLY_LIST, Uint8, Uint8, offsetof(ply::Face,nvts) };
	f_props_map["vertex_indices"] = f_props[0];
	std::vector<VertexWithMsure> vts;
	std::vector<internal::Edge> edges;
	std::vector<internal::Face> faces;

	ply::PLYreader ply_reader;
	cout << " before reading " << endl;
	if (ply_reader.read(inFile, v_props_map, e_props_map, f_props_map, vts, edges, faces) != ply::SUCCESS)
	{
		std::cout << "failed to read mesh file " << inFile << std::endl;
	}

	std::map<int, std::vector<int> > vertIndexEdgeIndices;
	for (int i = 0; i < edges.size(); i++) {
		if (vertIndexEdgeIndices.find(edges[i].v1) == vertIndexEdgeIndices.end()) {
			std::vector<int> edgeIndices = { i };
			vertIndexEdgeIndices[edges[i].v1] = edgeIndices;
		}
		else {
			std::vector<int> edgeIndices = vertIndexEdgeIndices[edges[i].v1];
			edgeIndices.push_back(i);
			vertIndexEdgeIndices[edges[i].v1] = edgeIndices;
		}
		if (vertIndexEdgeIndices.find(edges[i].v2) == vertIndexEdgeIndices.end()) {
			std::vector<int> edgeIndices = { i };
			vertIndexEdgeIndices[edges[i].v2] = edgeIndices;

		}
		else {
			std::vector<int> edgeIndices = vertIndexEdgeIndices[edges[i].v2];
			edgeIndices.push_back(i);
			vertIndexEdgeIndices[edges[i].v2] = edgeIndices;
		}
	}

	int highestRadEndPt = -1;
	int highestRadPt = 0;
	int highestYRadPt = -1;
	for (int i = 0; i < vts.size(); i++) {
		if (vertIndexEdgeIndices[i].size() == 1) {
			if (highestRadEndPt != -1) {
				if (vts[i].radius > vts[highestRadEndPt].radius) {
					highestRadEndPt = i;
				}
			}
			else {
				highestRadEndPt = i;
			}
		}

		if (vts[i].radius > vts[highestRadPt].radius) {
			highestRadPt = i;
		}

		if (highestYRadPt == -1) {
			if (vts[i].radius > upperRadiiThresh) {
				highestYRadPt = i;
			}
		}
		else {
			if (vts[i].radius > upperRadiiThresh) {
				if (vts[i].y > vts[highestYRadPt].y) {
					highestYRadPt = i;
				}
			}
		}
	}

	vector< vector<int> > vertexFaces;
	for (int i = 0; i < vts.size(); i++) {
		vector<int> vFaces;
		vertexFaces.push_back(vFaces);
	}
	for (int i = 0; i < faces.size(); i++) {
		vertexFaces[faces[i].verts[0]].push_back(i);
		vertexFaces[faces[i].verts[1]].push_back(i);
		vertexFaces[faces[i].verts[2]].push_back(i);

	}
	map<int, bool> visitedFaceVertices;
	for (int i = 0; i < faces.size(); i++) {
		for (int j = 0; j < 3; j++) {
			vector<int> connectedEdgeVts;
			if (visitedFaceVertices.find(faces[i].verts[j]) == visitedFaceVertices.end()) {
				std::queue<int> faceCompQ;
				faceCompQ.push(faces[i].verts[j]);
				vector<float> center = { 0.0, 0.0, 0.0, 0.0, 0.0 };
				while (!faceCompQ.empty()) {
					int currentVertex = faceCompQ.front();
					faceCompQ.pop();
					if (visitedFaceVertices.find(currentVertex) == visitedFaceVertices.end()) {
						visitedFaceVertices[currentVertex] = true;
						if (vertIndexEdgeIndices.find(currentVertex) != vertIndexEdgeIndices.end()) {
							connectedEdgeVts.push_back(currentVertex);
							center[0] += vts[currentVertex].x;
							center[1] += vts[currentVertex].y;
							center[2] += vts[currentVertex].z;
							center[3] = max(center[3], vts[currentVertex].radius);
							center[4] += vts[currentVertex].width;
						}


						vector<int> adjFaces = vertexFaces[currentVertex];
						for (int k = 0; k < adjFaces.size(); k++) {
							for (int l = 0; l < 3; l++) {
								if (faces[adjFaces[k]].verts[l] != currentVertex) {
									faceCompQ.push(faces[adjFaces[k]].verts[l]);
								}
							}

						}
					}
				}
				if (connectedEdgeVts.size() > 0) {
					center[0] /= connectedEdgeVts.size();
					center[1] /= connectedEdgeVts.size();
					center[2] /= connectedEdgeVts.size();
					center[3] = max((float)(lowerStemThresh + 2.0), center[3]);
					center[4] /= connectedEdgeVts.size();

					int newIndex = vts.size();
					for (int g = 0; g < connectedEdgeVts.size(); g++) {
						internal::Edge newEdge;
						newEdge.v1 = connectedEdgeVts[g];
						newEdge.v2 = newIndex;

						std::vector<int> edgeIndices = vertIndexEdgeIndices[connectedEdgeVts[g]];
						edgeIndices.push_back(edges.size());
						vertIndexEdgeIndices[connectedEdgeVts[g]] = edgeIndices;

						if (vertIndexEdgeIndices.find(vts.size()) == vertIndexEdgeIndices.end()) {
							vertIndexEdgeIndices[vts.size()] = { (int)edges.size() };
						}
						else {
							std::vector<int> edgeIndicesNew = vertIndexEdgeIndices[vts.size()];
							edgeIndicesNew.push_back(edges.size());
							vertIndexEdgeIndices[vts.size()] = edgeIndicesNew;
						}

						edges.push_back(newEdge);
					}

					VertexWithMsure newV;
					newV.x = center[0];
					newV.y = center[1];
					newV.z = center[2];
					newV.radius = center[3];
					newV.width = center[4];
					vts.push_back(newV);
				}
			}
		}
	}
	
	std::map<vector<float>, int> vertIndex;
	int maxRadiusVtIndex = 0;
	for (int i = 0; i < vts.size(); i++) {
		if (vts[i].radius > upperRadiiThresh) {
			vts[i].inDiameter = 1;
		}
		else {
			vts[i].inDiameter = -1;
		}
		if (vts[i].radius > vts[maxRadiusVtIndex].radius) {
			maxRadiusVtIndex = i;
		}
	}

	//Start stem search
	std::vector < std::tuple< std::vector<VertexWithMsure>, std::vector<int>, float> > potentialStemComponents;
	std::map<int, int> connectedVertexComponents;
	std::vector<int> vertComps;
	std::vector<int> edgeComps;
	//Find vertices with radius greater than upperRadiiThresh, and perform BFS to find high radius connected component starting from that vertex
	for (int i = 0; i < vts.size(); i++) {
		vertIndex[{vts[i].x, vts[i].y, vts[i].z}] = i; //Can ignore

		//If haven't visited this vertex yet and above upperRadiiThresh, begin new component
		if (connectedVertexComponents.find(i) == connectedVertexComponents.end()) {
			if (vts[i].radius > upperRadiiThresh) {

				std::vector<int> newVertComp;
				std::vector<int> newEdgeComp;

				//Flood fill one component by traversing edges along skeleton
				fillComponentIterative(connectedVertexComponents, i, vertIndexEdgeIndices, edges, vertComps, newEdgeComp, newVertComp, lowerRadiiThresh, vts);
				
				//Optional step: threshold the largest components by the number of vertices
				float length = getLengthOfComponent(newEdgeComp, edges, vts);

				//Length threshold
				if (length > compThreshSize) {

					//Add new connected component of vertices to component list
					std::vector<VertexWithMsure> vertPos;
					for (int j = 0; j < newVertComp.size(); j++) {
						vertPos.push_back(vts[newVertComp[j]]);
					}
					potentialStemComponents.push_back(make_tuple(vertPos, newEdgeComp, length));
					break;
				}
			}

		}
	}

	//Find high-radius connected component with the largest size
	std::sort(potentialStemComponents.begin(), potentialStemComponents.end(), [](auto const &t1, auto const &t2) {
		return get<2>(t1) > get<2>(t2);
	}
	);
	vector<int> stemEdges = get<1>(potentialStemComponents[0]);

	std::vector<VertexWithMsure> stemVertices;
	std::vector<internal::Edge> stemEdgesR;
	std::vector<internal::Face> stemFaces;


	std::map<int, int> oldToNewVtIndex;
	struct VertexProperties {
		int degree;
		int burnTime;
	};


	typedef adjacency_list < vecS, vecS, undirectedS, VertexProperties, property < edge_weight_t, int > > Graph;

	typedef graph_traits < Graph >::edge_descriptor Edge;

	typedef graph_traits < Graph >::vertex_descriptor Vertex;

	std::vector<E> stemEdgesReindexed;

	//vector<float> weights;
	float * weights = new float[stemEdges.size()];
	E* edgePairs = new E[stemEdges.size()];
	map<E, float> edgeWeightMap;
	map<int, int> stemToOrigVertexMapping;
	for (int i = 0; i < stemEdges.size(); i++) {
		if (oldToNewVtIndex.find(edges[stemEdges[i]].v1) == oldToNewVtIndex.end()) {
			oldToNewVtIndex[edges[stemEdges[i]].v1] = stemVertices.size();
			stemToOrigVertexMapping[stemVertices.size()] = edges[stemEdges[i]].v1;
			stemVertices.push_back(vts[edges[stemEdges[i]].v1]);
			stemVertices[stemVertices.size() - 1].width = 1;
		}
		if (oldToNewVtIndex.find(edges[stemEdges[i]].v2) == oldToNewVtIndex.end()) {
			oldToNewVtIndex[edges[stemEdges[i]].v2] = stemVertices.size();
			stemToOrigVertexMapping[stemVertices.size()] = edges[stemEdges[i]].v2;
			stemVertices.push_back(vts[edges[stemEdges[i]].v2]);
			stemVertices[stemVertices.size() - 1].width = 1;
		}
		internal::Edge newE = edges[stemEdges[i]];
		newE.v1 = oldToNewVtIndex[edges[stemEdges[i]].v1];
		newE.v2 = oldToNewVtIndex[edges[stemEdges[i]].v2];
		edgePairs[i] = E(newE.v1, newE.v2);
		stemEdgesR.push_back(newE);

		if (stemVertices[newE.v1].inDiameter == 1 || stemVertices[newE.v2].inDiameter == 1) {
			weights[i] = 0.0;
		}
		else {
			float avgRadius = (stemVertices[newE.v1].radius + stemVertices[newE.v2].radius) / 2.0;
			if (avgRadius < lowerStemThresh) {
				weights[i] = 100000;
			}
			else {
				weights[i] = gaussianFactor(avgRadius, 0.5);
			}
			edgeWeightMap[edgePairs[i]] = weights[i];
		}
	}
	ply::PLYwriter ply_writerS;
	string outFileNameS = "C:/Users/danzeng/Dropbox/sorghum/sorghumRegion.ply";

	std::map<std::string, PlyProperty> vert_propsS;

	vert_propsS["x"] = { (char*)"x", Float32, Float32, offsetof(VertexWithMsure, x), PLY_SCALAR, 0, 0, 0 };

	vert_propsS["y"] = { (char*)"y", Float32, Float32, offsetof(VertexWithMsure, y), PLY_SCALAR, 0, 0, 0 };

	vert_propsS["z"] = { (char*)"z", Float32, Float32, offsetof(VertexWithMsure, z), PLY_SCALAR, 0, 0, 0 };

	vert_propsS["radius"] = { (char*)"radius", Float32, Float32, offsetof(VertexWithMsure, radius), PLY_SCALAR, 0, 0, 0 };

	vert_propsS["bt2"] = { (char*)"bt2", Float32, Float32, offsetof(VertexWithMsure, width), PLY_SCALAR, 0, 0, 0 };

	vert_propsS["compType"] = { (char*)"compType", Float32, Float32, offsetof(VertexWithMsure, compType), PLY_SCALAR, 0, 0, 0 };
	vert_propsS["compType"] = { (char*)"compType", Float32, Float32, offsetof(VertexWithMsure, compType), PLY_SCALAR, 0, 0, 0 };

	std::map<std::string, PlyProperty> edge_propsS;

	edge_propsS["vertex1"] = { (char*)"vertex1", Int32, Int32, offsetof(ply::Edge, v1), PLY_SCALAR, 0, 0, 0 };

	edge_propsS["vertex2"] = { (char*)"vertex2", Int32, Int32, offsetof(ply::Edge, v2), PLY_SCALAR, 0, 0, 0 };

	std::map<std::string, PlyProperty> face_propsS;

	face_propsS["vertex_indices"] = {

		(char*)"vertex_indices", Int32, Int32, offsetof(ply::Face, verts),

		PLY_LIST, Uint8, Uint8, offsetof(ply::Face,nvts) };
	ply_writerS.write(outFileNameS.c_str(), true, true, true,

		vert_propsS, edge_propsS, face_propsS,

		stemVertices, stemEdgesR, stemFaces);

#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
	Graph g(vts.size());
	property_map<Graph, edge_weight_t>::type weightmap = get(edge_weight, g);
	for (std::size_t j = 0; j < stemEdges.size(); ++j) {
		Edge e; bool inserted;
		boost::tie(e, inserted) = add_edge(stemEdges[j].v1, stemEdges[j].v2s, g);
		weightmap[e] = weights[j];
	}
#else

	Graph g(edgePairs, edgePairs + stemEdges.size(), weights, stemVertices.size());
#endif
	std::vector < Edge > spanning_tree;
	map<int, int> vertBurnTimes;
	kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));

	stemEdges.clear(); stemEdges.shrink_to_fit();

	float * mstWeights = new float[spanning_tree.size()];
	E * spanningTreeEdges = new E[spanning_tree.size()];


	vector< vector<E> > mstVertEdges(vts.size());
	vector<internal::Edge> mstEdges;
	for (int i = 0; i < spanning_tree.size(); i++) {
		E edge = E(source(spanning_tree[i], g), target(spanning_tree[i], g));
		spanningTreeEdges[i] = edge;
		mstWeights[i] = edgeWeightMap[edge];
		mstVertEdges[source(spanning_tree[i], g)].push_back(edge);
		mstVertEdges[target(spanning_tree[i], g)].push_back(edge);
		internal::Edge e;
		e.v1 = source(spanning_tree[i], g); e.v2 = target(spanning_tree[i], g);
		mstEdges.push_back(e);
	}


	//Minimum spanning tree of high radius region -> this is a boost object
	Graph mst(spanningTreeEdges, spanningTreeEdges + spanning_tree.size(), mstWeights, stemVertices.size());
	ply_writerS.write("C:/Users/danzeng/Dropbox/sorghum/sorghumMST.ply", true, true, true,

		vert_propsS, edge_propsS, face_propsS,

		stemVertices, mstEdges, stemFaces);
	//Temporary structure to keep track of edges during burning process
	Graph subGraph = mst;

	//Start burning process
	bool burnable = true;
	int burnRound = 0;
	vector< vector<E> > burnRoundMapping(100000);
	vector< vector<int> > burnRoundMappingVts(100000);
	vector<int> endPts;
	while (burnable) {
		vector<Edge> subgraphEdges;
		burnable = false;
		int burnableEdges = 0;
		vector<Edge> burnedEdges;
		for (int i = 0; i < spanning_tree.size(); i++) {

			E edge = E(source(spanning_tree[i], subGraph), target(spanning_tree[i], subGraph));
			spanningTreeEdges[i] = edge;

			//Optional: I assigned a special weight for each edge based on the surrounding radii, but you can just use the average radius of the edge vertices
			mstWeights[i] = edgeWeightMap[edge];

			//Implicitly perform burning by only preserving edges that are not part of endpoints for the next burning iteration
			if (subGraph.m_vertices[source(spanning_tree[i], subGraph)].m_out_edges.size() > 1 && subGraph.m_vertices[target(spanning_tree[i], subGraph)].m_out_edges.size() > 1) {
				subgraphEdges.push_back(spanning_tree[i]);
			}
			else {
				//Edge was burned this round: record the burn time of each edge
				burnRoundMapping[burnRound].push_back(edge);
				
				burnableEdges += 1;

				//Record when vertices are burned as well
				if (subGraph.m_vertices[source(spanning_tree[i], subGraph)].m_out_edges.size() == 1) {
					vertBurnTimes[source(spanning_tree[i], subGraph)] = burnRound;
					burnable = true;
					burnRoundMappingVts[burnRound].push_back(source(spanning_tree[i], subGraph));
				}
				if (subGraph.m_vertices[target(spanning_tree[i], subGraph)].m_out_edges.size() == 1) {
					vertBurnTimes[target(spanning_tree[i], subGraph)] = burnRound;
					burnable = true;
					burnRoundMappingVts[burnRound].push_back(target(spanning_tree[i], subGraph));
				}
				burnedEdges.push_back(spanning_tree[i]);

			}
		}
		//For debugging
		cout << "burnable edges " << burnableEdges << endl;

		if (subgraphEdges.size() <= 1) {
			burnable = false;
		}
		spanning_tree = subgraphEdges;
		//Remove burned edges from the graph
		for (int i = 0; i < burnedEdges.size(); i++) {
			remove_edge(source(burnedEdges[i], subGraph), target(burnedEdges[i], subGraph), subGraph);
		}
		burnRound += 1;

	}

	//Start inverse burn
	Graph diameterGraph;

	vector<E> diameterEdgeVec = burnRoundMapping[burnRound - 1];
	
	burnRound -= 1;

	//Find the vertices which were burned this round. These are the endpoints of the current iteration to expand the stem from
	vector<int> seedVertices = burnRoundMappingVts[burnRound];

	//For recording which vertices are in the stem (The stem is the graph diameter of the high radius region
	map<int, bool> vertexInDiameter;
	vector<int> diameterVts;

	vector< vector<int> > branchEdges;
	int lowestSeedVert = 0;
	while (burnRound > 0) {
		cout << "burn round " << burnRound << endl;

		//Record the vertices and edges which are currently in the stem
		E * diameterEdges = new E[diameterEdgeVec.size()];
		float * diameterWeights = new float[diameterEdgeVec.size()];
		for (int i = 0; i < diameterEdgeVec.size(); i++) {
			diameterEdges[i] = diameterEdgeVec[i];

			//Optional part to weight edges based upon neighboring radii -> you dont need to include this
			diameterWeights[i] = edgeWeightMap[diameterEdgeVec[i]];
		}


		vector<int> nextSeedVertices;
		//Get edges for bordering vertices
		for (int i = 0; i < seedVertices.size(); i++) {

			//Add to stem
			vertexInDiameter[seedVertices[i]] = true;
			diameterVts.push_back(seedVertices[i]);
			//Find edges adjacent to endpoints of stem so far
			vector<E> vertEdges = mstVertEdges[seedVertices[i]];
			float bestRadius = -1;
			int bestSeed = -1;
			E bestEdge;
			//Choose edge whose other endpoint has highest radius, and whose burn time is one less than the current one
			for (int j = 0; j < vertEdges.size(); j++) {
				E edge = vertEdges[j];
				int nextSeed = -1;
				if (vertBurnTimes[edge.first] == burnRound - 1) {
					nextSeed = edge.first;
				}
				if (vertBurnTimes[edge.second] == burnRound - 1) {
					nextSeed = edge.second;
				}
				if (nextSeed != -1) {
					if (vertexInDiameter.find(nextSeed) == vertexInDiameter.end()) {

						if (stemVertices[nextSeed].inDiameter == 1) {

							if (stemVertices[nextSeed].radius > bestRadius)
							{
								bestRadius = edgeWeightMap[edge];
								bestSeed = nextSeed;
								bestEdge = edge;
							}
						}
						else {
							//getRangeScore is just a scoring function for which vertex to choose to add to the stem. Just replace this with the vertex radius
							if (getRangeScore(mstVertEdges, nextSeed, stemVertices, vertexInDiameter, 10) > bestRadius) {
								bestRadius = edgeWeightMap[edge];
								bestSeed = nextSeed;
								bestEdge = edge;
							}
						}

					}
				}

			}
			if (bestSeed > -1) {
				//Find edges for decremented burn time
				nextSeedVertices.push_back(bestSeed);
				diameterEdgeVec.push_back(bestEdge);

			}

		}

		//Decrement burn time
		seedVertices = nextSeedVertices;
		burnRound -= 1;
	}
	map<int, int> newToOldIndexS;
	vector<VertexWithMsure> stemAndDiameterVerticesS;
	vector<internal::Edge> stemAndDiameterEdgesOutS;
	for (int i = 0; i < diameterEdgeVec.size(); i++) {
		int origVtsIndex1 = stemToOrigVertexMapping[diameterEdgeVec[i].first];
		if (newToOldIndexS.find(origVtsIndex1) == newToOldIndexS.end()) {

			newToOldIndexS[origVtsIndex1] = stemAndDiameterVerticesS.size();
			vts[origVtsIndex1].compType = 1;
			stemAndDiameterVerticesS.push_back(vts[origVtsIndex1]);
		}
		int origVtsIndex2 = stemToOrigVertexMapping[diameterEdgeVec[i].second];
		if (newToOldIndexS.find(origVtsIndex2) == newToOldIndexS.end()) {
			newToOldIndexS[origVtsIndex2] = stemAndDiameterVerticesS.size();
			vts[origVtsIndex2].compType = 1;
			stemAndDiameterVerticesS.push_back(vts[origVtsIndex2]);
		}
		internal::Edge e;
		e.v1 = origVtsIndex1;
		e.v2 = origVtsIndex2;
		stemAndDiameterEdgesOutS.push_back(e);
	}
	ply_writerS.write("C:/Users/danzeng/Dropbox/sorghum/sorghumStem.ply", true, true, true,

		vert_propsS, edge_propsS, face_propsS,

		vts, stemAndDiameterEdgesOutS, stemFaces);
	
	if (oldToNewVtIndex.find(highestRadEndPt) == oldToNewVtIndex.end()) {
		//Highest radius not even in MST, so not even in stem
		oldToNewVtIndex[highestRadEndPt] = stemVertices.size();
		stemToOrigVertexMapping[stemVertices.size()] = highestRadEndPt;
		stemVertices.push_back(vts[highestRadEndPt]);
	}
	if (vertexInDiameter.find(oldToNewVtIndex[highestRadEndPt]) == vertexInDiameter.end()) {
		int currentVtx = oldToNewVtIndex[highestRadEndPt];
		map<int, bool> connectionVisited;
		std::queue<int> connectionsToVisit;
		connectionsToVisit.push(currentVtx);
		map<int, int> pathParents;
		while (vertexInDiameter.find(currentVtx) == vertexInDiameter.end() && !connectionsToVisit.empty()) {
			currentVtx = connectionsToVisit.front();
			connectionsToVisit.pop();
			int currentVtxOrig = stemToOrigVertexMapping[currentVtx];
			if (connectionVisited.find(currentVtxOrig) == connectionVisited.end()) {
				connectionVisited[currentVtxOrig] = true;
				vector<int> nEdges = vertIndexEdgeIndices[currentVtxOrig];

				for (int j = 0; j < nEdges.size(); j++) {
					int nVtx = -1;
					if (edges[nEdges[j]].v1 != currentVtxOrig) {
						nVtx = edges[nEdges[j]].v1;
					}
					if (edges[nEdges[j]].v2 != currentVtxOrig) {
						nVtx = edges[nEdges[j]].v2;
					}
					if (edges[nEdges[j]].v2 != currentVtxOrig && edges[nEdges[j]].v1 != currentVtxOrig) {
						cout << "bug, both edges are different than current vtx " << endl;
					}
					if (nVtx != -1) {


						if (oldToNewVtIndex.find(nVtx) == oldToNewVtIndex.end()) {
							oldToNewVtIndex[nVtx] = stemVertices.size();
							stemToOrigVertexMapping[stemVertices.size()] = nVtx;
							stemVertices.push_back(vts[nVtx]);
						}
						if (connectionVisited.find(nVtx) == connectionVisited.end()) {
							pathParents[nVtx] = currentVtxOrig;
							connectionsToVisit.push(oldToNewVtIndex[nVtx]);
						}



					}
				}
			}

		}
		if (vertexInDiameter.find(currentVtx) != vertexInDiameter.end()) {
			while (currentVtx != oldToNewVtIndex[highestRadEndPt]) {
				if (vertexInDiameter.find(currentVtx) == vertexInDiameter.end()) {
					diameterVts.push_back(currentVtx);
					vertexInDiameter[currentVtx] = true;
				}
				int currentVtxOrig = stemToOrigVertexMapping[currentVtx];
				int parentOrig = pathParents[currentVtxOrig];
				if (parentOrig == currentVtxOrig) {
					break;
				}
				E newDiamE(currentVtx, oldToNewVtIndex[parentOrig]);
				mstVertEdges[currentVtx].push_back(newDiamE);

				diameterEdgeVec.push_back(newDiamE);
				currentVtx = oldToNewVtIndex[parentOrig];
			}
			if (vertexInDiameter.find(oldToNewVtIndex[highestRadEndPt]) == vertexInDiameter.end()) {
				diameterVts.push_back(oldToNewVtIndex[highestRadEndPt]);
				vertexInDiameter[oldToNewVtIndex[highestRadEndPt]] = true;
			}
		}

	}

	
	
	//Find distances from highest radius point to all other points
	cout << "Forcing highest radius endpoint to be on stem" << endl;
	int startVtx = highestRadEndPt;
	std::queue<int> diamSearchQ;
	diamSearchQ.push(oldToNewVtIndex[startVtx]);
	map<int, bool> diamVisited;
	map<int, float> parentDistDiam;
	parentDistDiam[oldToNewVtIndex[startVtx]] = 0.0;
	int diamVtsReached = 0;
	while (!diamSearchQ.empty()) {
		int currentVtx = diamSearchQ.front();
		diamSearchQ.pop();
		if (diamVisited.find(currentVtx) == diamVisited.end()) {
			diamVisited[currentVtx] = true;
			//if (vertexInDiameter.find(currentVtx) != vertexInDiameter.end()) {
			diamVtsReached += 1;
			int currentVtxOld = stemToOrigVertexMapping[currentVtx];
			vector<int> neighborEdgesOldIndexing = vertIndexEdgeIndices[currentVtxOld];
			for (int i = 0; i < neighborEdgesOldIndexing.size(); i++) {
				int nVtx = -1;
				if (edges[neighborEdgesOldIndexing[i]].v1 != currentVtxOld) {
					nVtx = edges[neighborEdgesOldIndexing[i]].v1;
				}
				if (edges[neighborEdgesOldIndexing[i]].v2 != currentVtxOld) {
					nVtx = edges[neighborEdgesOldIndexing[i]].v2;
				}
				if (nVtx != -1) {
					if (oldToNewVtIndex.find(nVtx) != oldToNewVtIndex.end()) {
						int stemNIndex = oldToNewVtIndex[nVtx];
						if (diamVisited.find(stemNIndex) == diamVisited.end()) {
							//if neighbor also in diameter, keep traversing
							parentDistDiam[stemNIndex] = parentDistDiam[currentVtx] + euclideanDistance(stemVertices[currentVtx], stemVertices[stemNIndex]);

							diamSearchQ.push(stemNIndex);
						}
					}
				}
			}

			//}
		}
	}
	
	int ct = 0;
	for (int i = 0; i < diameterVts.size(); i++) {
		vector<E> diamEdges = mstVertEdges[diameterVts[i]];
		map< vector<int>, bool> currentVEdgeMap;
		vector<E> newEdges;
		for (int j = 0; j < diamEdges.size(); j++) {
			E e = diamEdges[j];
			if (currentVEdgeMap.find({ e.first, e.second }) == currentVEdgeMap.end()) {
				newEdges.push_back(e);
				currentVEdgeMap[{ e.first, e.second }] = true;
				currentVEdgeMap[{ e.second, e.first }] = true;

			}
			else {
				ct += 1;
			}
		}
		mstVertEdges[diameterVts[i]] = diamEdges;
	}


	double * diameterValues = new double[diameterVts.size() * 3];
	float centerX = 0.0;
	float centerY = 0.0;
	float centerZ = 0.0;
	int lowestSeed = 0;
	for (int i = 0; i < diameterVts.size(); i++) {
		diameterValues[3 * i] = stemVertices[diameterVts[i]].x;
		diameterValues[(3 * i) + 1] = stemVertices[diameterVts[i]].y;
		diameterValues[(3 * i) + 2] = stemVertices[diameterVts[i]].z;

		centerX += stemVertices[diameterVts[i]].x;
		centerY += stemVertices[diameterVts[i]].y;
		centerZ += stemVertices[diameterVts[i]].z;

		if (stemVertices[diameterVts[i]].radius > stemVertices[lowestSeed].radius) {
			lowestSeed = diameterVts[i];
		}
	}
	centerX /= diameterVts.size();
	centerY /= diameterVts.size();
	centerZ /= diameterVts.size();
	vector<float> center = { centerX, centerY, centerZ };
	
	spanning_tree.clear();

	//Output branch measures as they are discovered
	std::ofstream outStream;
	outStream.open(outMeasureFile);
	if (!outStream.is_open()) {
		std::cout << "not open" << std::endl;
		return 1;
	}

	outStream << "junction x , junction y, junction z, branch length, straight-line distance, emergence angle, tip angle, PCA comp 1 x, PCA comp 1 y, PCA comp z, PCA comp 2 x, PCA comp 2 y, PCA comp 2 z, PCA comp 3 x, PCA comp 3 y, PCA comp 3 z, seed to junction distance" << endl;

	map<int, bool> origVertexInDiameter;
	vector<bool> vertexInDiameterVec;
	for (int i = 0; i < vts.size(); i++) {
		vertexInDiameterVec.push_back(false);
	}
	vector<float> stemCentroid = { 0.0,0.0,0.0 };
	for (const auto &myPair : vertexInDiameter) {
		origVertexInDiameter[stemToOrigVertexMapping[myPair.first]] = true;
		vertexInDiameterVec[stemToOrigVertexMapping[myPair.first]] = true;
		stemCentroid[0] += vts[stemToOrigVertexMapping[myPair.first]].x;
		stemCentroid[1] += vts[stemToOrigVertexMapping[myPair.first]].y;
		stemCentroid[2] += vts[stemToOrigVertexMapping[myPair.first]].z;
	}
	stemCentroid[0] /= diameterVts.size();
	stemCentroid[1] /= diameterVts.size();
	stemCentroid[2] /= diameterVts.size();

	vector < vector<int> > vertEdgesVec;
	for (int i = 0; i < vts.size(); i++) {
		vector<int> nVec;
		vertEdgesVec.push_back(nVec);
	}
	for (const auto &myPair : vertIndexEdgeIndices) {
		vertEdgesVec[myPair.first] = myPair.second;
	}
	vector<int> lastLoop;
	map<int, bool> visited = origVertexInDiameter;
	map<int, int> branchEndToStartMapping;
	int lowestIndex = 0;
	vector< std::tuple<int, float> > junctionDistances;
	float distTotal = 0.0;
	map<int, float> junctionDistToLowestSeed;
	vector<float> seedJunctionDistances;
	float firstPrimaryBranchLength = 0.0;
	int firstBranchIndex = -1;

	vector<std::tuple<int, float> > diameterVtsTuples;
	for (int i = 0; i < diameterVts.size(); i++) {
		diameterVtsTuples.push_back(std::make_tuple(diameterVts[i], parentDistDiam[diameterVts[i]]));
		//diameterVtsTuples.push_back(std::make_tuple(diameterVts[i], ));
	}

	sort(diameterVtsTuples.begin(), diameterVtsTuples.end(), sortbysec);

	cout << "diameterVtsTuples " << diameterVtsTuples.size() << endl;
	//sort from stem bottom to stem top
	diameterVts.clear();
	for (int i = 0; i < diameterVtsTuples.size(); i++) {
		diameterVts.push_back(get<0>(diameterVtsTuples[i]));
	}
	vector<int> visitedVts;
	for (int i = 0; i < vts.size(); i++) {
		visitedVts.push_back(-1);
	}
	for (const auto &myPair : visited) {
		visitedVts[myPair.first] = 1;
	}
	float avgTipAngle = 0.0;
	float avgEmergenceAngle = 0.0;
	float avgBranchLength = 0.0;
	vector<float> stemVec = { stemCentroid[0] - vts[highestYRadPt].x, stemCentroid[1] - vts[highestYRadPt].y , stemCentroid[2] - vts[highestYRadPt].z };
	normalize(stemVec);
	for (int i = 0; i < diameterVts.size(); i++) {
		junctionDistToLowestSeed[diameterVts[i]] = distTotal;
		if (i + 1 < diameterVts.size()) {
			distTotal += euclideanDistance(stemVertices[diameterVts[i]], stemVertices[diameterVts[i + 1]]);
		}
		vector<E> vertEdges = mstVertEdges[diameterVts[i]];
		cout << "At stem vertex number " << i << " of " << diameterVts.size() << endl;
		if (vertEdges.size() > 2) {
			for (int j = 0; j < vertEdges.size(); j++) {
				E edge = vertEdges[j];
				int junctionIndex = -1;
				if (edge.first == diameterVts[i]) {
					if (origVertexInDiameter.find(edge.second) == origVertexInDiameter.end()) {
						junctionIndex = stemToOrigVertexMapping[edge.second];
					}
				}
				if (edge.second == diameterVts[i]) {
					if (origVertexInDiameter.find(edge.first) == origVertexInDiameter.end()) {
						junctionIndex = stemToOrigVertexMapping[edge.first];
					}
				}
				if (junctionIndex != -1) {
					vector< vector<int> > seedBranches;
					vector<float> emergenceAngles;
					vector<float> tipAngles;
					vector<float> branchLengths;
					vector<float> tipLengths;
					//vector< vector<float> > pcas;
					
					seedBranches = buildBranchesFromVertexForwardBFS(junctionIndex, vertexInDiameterVec, edges, vertEdgesVec, vts, visitedVts, branchSizeMin, curvatureWindow, maxBranchLength, center, lastLoop, radiusTol, emergenceAngleThresh, tipAngleThresh, tortuosityThresh, stemVec, emergenceAngles, tipAngles, branchLengths, tipLengths, emergenceLowerRadius, emergeWindow);

						
					for (int k = 0; k < seedBranches.size(); k++) {
						vector<int> seedBranchEdges = seedBranches[k];

						float dist = branchLengths[k];

						bool emergenceAngleWithinRange = true;
						float farthestEndPtDist = tipLengths[k];

						VertexWithMsure vt = vts[stemToOrigVertexMapping[diameterVts[i]]];

						float emergenceAngle = emergenceAngles[k];
						float tipAngle = tipAngles[k];

						//Keep track of distances to bottom of stem, to find internode lengths at end 
						seedJunctionDistances.push_back(parentDistDiam[diameterVts[i]]);

						//Update first primary branch length if necessary
						if (firstBranchIndex == -1) {
							firstBranchIndex = diameterVts[i];
							firstPrimaryBranchLength = dist;
						}
						else {
							if (parentDistDiam[diameterVts[i]] < parentDistDiam[firstBranchIndex]) {
								firstBranchIndex = diameterVts[i];
								firstPrimaryBranchLength = dist;
							}
						}
						float tortuosity;
						if (farthestEndPtDist > 0.0) {
							tortuosity = dist / farthestEndPtDist;
						}
						else {
							tortuosity = 10000.0;
						}
						cout << " branch statistics  " << vt.x << "," << vt.y << "," << vt.z << "," << dist << "," << farthestEndPtDist << "," << emergenceAngle << "," << tipAngle << "," << parentDistDiam[diameterVts[i]] << endl;

						outStream << vt.x << "," << vt.y << "," << vt.z << "," << dist << "," << farthestEndPtDist << "," << emergenceAngle << "," << tipAngle << "," << parentDistDiam[diameterVts[i]] << endl;

						emergenceAngleWithinRange = true;
						branchEdges.push_back(seedBranchEdges);
						cout << "added branch of size " << seedBranchEdges.size() << endl;
						avgBranchLength += dist;
						avgEmergenceAngle += emergenceAngle;
						avgTipAngle += tipAngle;

						//Need to move this within branch visiting code
						for (int k = 0; k < seedBranchEdges.size(); k++) {
							if (seedBranchEdges[k] < edges.size()) {
								visited[edges[seedBranchEdges[k]].v1] = 1;
								visited[edges[seedBranchEdges[k]].v2] = 1;
							}
						}
							
						

					}
				}
			}
		}
	}
		int branchN = branchEdges.size();
	if (branchEdges.size() == 0) {
		branchN = 10000000;

	}
	avgBranchLength /= branchN;
	avgTipAngle /= branchN;
	avgEmergenceAngle /= branchN;

	outStream << "Average branch length," << avgBranchLength << endl;
	outStream << "Average emergence angle," << avgEmergenceAngle << endl;
	outStream << "Average tip angle," << avgTipAngle << endl;

	if (seedJunctionDistances.size() > 0) {
		//Find first and 2nd longest internode lengths
		std::sort(seedJunctionDistances.begin(), seedJunctionDistances.end());
		vector<float> internodeDistances;
		for (int i = 0; i < seedJunctionDistances.size() - 1; i++) {
			internodeDistances.push_back(seedJunctionDistances[i + 1] - seedJunctionDistances[i]);
		}
		std::sort(internodeDistances.begin(), internodeDistances.end(), greater<int>());
		outStream << "First longest internode length, " << internodeDistances[0] << endl;
		outStream << "Second longest internode length, " << internodeDistances[1] << endl;
		outStream << "First primary branch length, " << firstPrimaryBranchLength << endl;
	}
	outStream << "Number of primary branches, " << branchN << endl;
	outStream.close();

	cout << "reindexing diameter vertices " << endl;
	map<int, int> newToOldIndex;
	vector<VertexWithMsure> stemAndDiameterVertices;
	vector<internal::Edge> stemAndDiameterEdgesOut;
	for (int i = 0; i < diameterEdgeVec.size(); i++) {
		int origVtsIndex1 = stemToOrigVertexMapping[diameterEdgeVec[i].first];
		if (newToOldIndex.find(origVtsIndex1) == newToOldIndex.end()) {

			newToOldIndex[origVtsIndex1] = stemAndDiameterVertices.size();
			vts[origVtsIndex1].compType = 1;
			stemAndDiameterVertices.push_back(vts[origVtsIndex1]);
		}
		int origVtsIndex2 = stemToOrigVertexMapping[diameterEdgeVec[i].second];
		if (newToOldIndex.find(origVtsIndex2) == newToOldIndex.end()) {
			newToOldIndex[origVtsIndex2] = stemAndDiameterVertices.size();
			vts[origVtsIndex2].compType = 1;
			stemAndDiameterVertices.push_back(vts[origVtsIndex2]);
		}
		internal::Edge e;
		e.v1 = newToOldIndex[origVtsIndex1];
		e.v2 = newToOldIndex[origVtsIndex2];
		stemAndDiameterEdgesOut.push_back(e);
	}
	diameterEdgeVec.clear();

	cout << "number branches " << branchEdges.size() << endl;
	for (int i = 0; i < branchEdges.size(); i++) {
		vector<int> currentBranch = branchEdges[i];
		for (int j = 0; j < currentBranch.size(); j++) {
			if (newToOldIndex.find(edges[currentBranch[j]].v1) == newToOldIndex.end()) {

				newToOldIndex[edges[currentBranch[j]].v1] = stemAndDiameterVertices.size();
				vts[edges[currentBranch[j]].v1].compType = i + 2;
				stemAndDiameterVertices.push_back(vts[edges[currentBranch[j]].v1]);
			}
			if (newToOldIndex.find(edges[currentBranch[j]].v2) == newToOldIndex.end()) {

				newToOldIndex[edges[currentBranch[j]].v2] = stemAndDiameterVertices.size();
				vts[edges[currentBranch[j]].v2].compType = i + 2;
				stemAndDiameterVertices.push_back(vts[edges[currentBranch[j]].v2]);
			}
			internal::Edge e;
			e.v1 = newToOldIndex[edges[currentBranch[j]].v1];
			e.v2 = newToOldIndex[edges[currentBranch[j]].v2];
			stemAndDiameterEdgesOut.push_back(e);
		}
	}

	vector<internal::Face> stemAndDiameterFaces;

	ply::PLYwriter ply_writer;
	string outFileName = outFile;

	std::map<std::string, PlyProperty> vert_props;

	vert_props["x"] = { (char*)"x", Float32, Float32, offsetof(VertexWithMsure, x), PLY_SCALAR, 0, 0, 0 };

	vert_props["y"] = { (char*)"y", Float32, Float32, offsetof(VertexWithMsure, y), PLY_SCALAR, 0, 0, 0 };

	vert_props["z"] = { (char*)"z", Float32, Float32, offsetof(VertexWithMsure, z), PLY_SCALAR, 0, 0, 0 };

	vert_props["radius"] = { (char*)"radius", Float32, Float32, offsetof(VertexWithMsure, radius), PLY_SCALAR, 0, 0, 0 };

	vert_props["bt2"] = { (char*)"bt2", Float32, Float32, offsetof(VertexWithMsure, width), PLY_SCALAR, 0, 0, 0 };

	vert_props["compType"] = { (char*)"compType", Float32, Float32, offsetof(VertexWithMsure, compType), PLY_SCALAR, 0, 0, 0 };
	vert_props["compType"] = { (char*)"compType", Float32, Float32, offsetof(VertexWithMsure, compType), PLY_SCALAR, 0, 0, 0 };

	std::map<std::string, PlyProperty> edge_props;

	edge_props["vertex1"] = { (char*)"vertex1", Int32, Int32, offsetof(ply::Edge, v1), PLY_SCALAR, 0, 0, 0 };

	edge_props["vertex2"] = { (char*)"vertex2", Int32, Int32, offsetof(ply::Edge, v2), PLY_SCALAR, 0, 0, 0 };

	std::map<std::string, PlyProperty> face_props;

	face_props["vertex_indices"] = {

		(char*)"vertex_indices", Int32, Int32, offsetof(ply::Face, verts),

		PLY_LIST, Uint8, Uint8, offsetof(ply::Face,nvts) };
	ply_writer.write(outFileName.c_str(), true, true, true,

		vert_props, edge_props, face_props,

		stemAndDiameterVertices, stemAndDiameterEdgesOut, stemAndDiameterFaces);

	return 0;
}



