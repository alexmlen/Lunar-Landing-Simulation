// CS 134 - Spring 2019
// Copyright (c) Kevin Smith March 2019
//
//

//  create the Octree "numLevels" deep from the mesh. Will return when
//  "numLevels" have been created or when all leave nodes contain one 
//  vertex, whichever comes first.
//
void Octree::create(const ofMesh & mesh, int numLevels) {
	// initialize octree structure
	//
	this->mesh = mesh;

    // initialize the firt root node (level 0)
	// it contains all vertex indices from the mesh
	// 
	int level = 0;
	root.box = meshBounds(mesh);
	for (int i = 0; i < mesh.getNumVertices(); i++) {
		root.points.push_back(i);
	}

	float t1 = ofGetElapsedTimeMillis();

	// recursively buid octree (starting at level 1)
	//
	level++;
	subdivide(root, numLevels, level);
	float t2 = ofGetElapsedTimeMillis();
	cout << "Time to Build Octree: " << t2 - t1 << " milliseconds" << endl;

}

//  primary recursive function to build octree.
//  algorithm:
//  1) return when the level is numLevels or greater
//  2) subdivide the current node box into 8 boxes
//  3) for each child box:
//      (a) distribute points in parent node to child node
//      (b) if there are points in child node add it to parent's children
//      (c) if a child has more than one point
//            recursively call subdivide on child
//
void Octree::subdivide(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	vector<Box> boxList;
	subDivideBox8(node.box, boxList);
	level++;
	for (int i = 0; i < boxList.size(); i++) {
		TreeNode child;
		int count = getMeshPointsInBox(node.points, boxList[i], child.points);
		if (count > 0) {
			child.box = boxList[i];
			node.children.push_back(child);
			if (count > 1) {
				subdivide(node.children[node.children.size() - 1], numLevels, level);
			}
		}
	}
}