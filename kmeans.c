/* 	K-means algorithm implementation 2020			*
 * 	MYE035-Computational Intelligence (elective course)	*
 * 	Angeliki Kalliri (contributor)				*/

// TODO: change the text file @ line 13 if you have to

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define M 5							// define the clusters  
#define testFile "Test.txt"			// text file with the examples (change this with your file)

// Information for the data with the examples' coordinates from the text file
float **data;
int row = 10;

// Information for the centres of the clusters
float centroids[M][2];
float centroid_x;
float centroid_y;
int closest_centroid = 0;	// information of the chosen centroid at the moment

// Auxiliary array that keeps the centroids at the end of each previous loop
float newCentroids[M][2];

// Auxiliary array for new assingment of the centroids
float **array;
int rows = 1;

// Create M-files to store each clusters' points closer to their centre 
FILE *fp[M];
char filename[10];


// Read the file with the examples and fill dynamically the data[][]
void ReadData(char nameOfFile[]) {
	FILE *file;
	char buffer[50], *s;
	char token[] = ",";
	int i, j, rows_before;
	int index_i = 0, index_j = 0, col = 2;

	// Create dynamic 2D array for the data (rows) that will be read 
	data = (float **) malloc(row*sizeof(float *));

	if (data == NULL) {
		exit(1);
	}

	// Now create the number of columns (for each row)
	for (i = 0; i < row; i++) {
		*(data + i) = (float *) malloc(col*sizeof(float));
		if (data[i] == NULL) {
			exit(1);
		}
	}

	if ((file = fopen(nameOfFile, "r")) == NULL) {
		exit(1);
	}

	// buffer gets the comma as well from the .txt
	while (fscanf(file, "%s", buffer) != EOF) {
		// Create more space in the data[][] (before it overflows)
		if (index_i == row) {
			rows_before = row;
			row = row + 1;	// add one more line, all the rows are used
			data = (float **) realloc(data, row*sizeof(float **));
			for (i = 0; i < rows_before; i++) {
				*(data + i) = (float *) realloc(*(data + i), row*sizeof(float));
				if (data[i] == NULL) {
					exit(1);
				}
			}	
			for (i = rows_before; i < row; i++) {
				*(data + i) = (float *) malloc(row*sizeof(float));
				if (data[i] == NULL) {
					exit(1);
				}
			}		
		}

		// s seperates the comma and gets the number only
		s = strtok(buffer, token);
		if (s != NULL){
			data[index_i][index_j] = atof(s);
			if (index_j == 0) {
				index_j++;
			}
			else {
				index_j--;
				index_i++;
			}
		}
	} 

	fclose(file); 
}


// Define the first centroids randomly
void defineCentroids() {
	int i, j; 
	srand ( time(NULL) );

	for (i = 0; i < M; i++) {
		int randomIndex = rand() % row;
		centroid_x = data[randomIndex][0];
		centroid_y = data[randomIndex][1];
		centroids[i][0] = centroid_x;
		centroids[i][1] = centroid_y;

		printf("Centroid %d: x = %f  y = %f\n", i + 1, centroids[i][0], centroids[i][1]);

		// Delete the centroid that was selected from tha data 
		/*row = row - 1;
		for (j = randomIndex; j < row; j++){
			data[j][0] = data[j+1][0];
			data[j][1] = data[j+1][1];
		}*/
	}

	// Add the initialized cetroids to the M-cluster files
	for (i = 0; i < M; i++) {
		sprintf(filename, "Cluster%d.txt", i + 1);
    	fp[i + 1] = fopen(filename, "w");
    	fprintf(fp[i + 1], "%f, %f\n", centroids[i][0], centroids[i][1]);
    	fclose(fp[i+1]);
	} 
} 


// Compute the Euclidean distance
float euclideanDistance(float x1, float y1, float x2, float y2) {
	float distance;
	distance = sqrt( pow(x1 - x2, 2) + pow(y1 - y2, 2) );
	return distance;
}


// Read the clusters text files and fill dynamically the auxiliary array[][]
void arrayCreation(char nameOfFile[]){
	FILE *file;
	char buffer[50], *s;
	char token[] = ",";
	int i, j, rows_before;
	int index_i = 0, index_j = 0, columns = 2;

	// Create dynamic 2D array for the data (rows) that will be read 
	array = (float **) malloc(rows*sizeof(float *));

	if (array == NULL) {
		exit(1);
	}

	// Now create the number of columns (for each row)
	for (i = 0; i < rows; i++) {
		*(array + i) = (float *) malloc(columns*sizeof(float));
		if (array[i] == NULL) {
			exit(1);
		}
	}

	if ((file = fopen(nameOfFile, "r")) == NULL) {
		exit(1);
	}

	// buffer gets the comma as well from the .txt
	while (fscanf(file, "%s", buffer) != EOF) {
		// Create more space in the data[][] (before it overflows)
		if (index_i == rows) {
			rows_before = rows;
			rows = rows + 1;	// add one more line, all the rows are used
			array = (float **) realloc(array, rows*sizeof(float **));
			for (i = 0; i < rows_before; i++) {
				*(array + i) = (float *) realloc(*(array + i), rows*sizeof(float));
				if (array[i] == NULL) {
					exit(1);
				}
			}	
			for (i = rows_before; i < rows; i++) {
				*(array + i) = (float *) malloc(rows*sizeof(float));
				if (array[i] == NULL) {
					exit(1);
				}
			}		
		}

		// s seperates the comma and gets the number only
		s = strtok(buffer, token);
		if (s != NULL) {
			array[index_i][index_j] = atof(s);
			if (index_j == 0) {
				index_j++;
			}
			else {
				index_j--;
				index_i++;
			}
		}
	} 

	fclose(file); 
}


// Calculate the new centroid by the mean of all the points in each cluster
void newCentroidAssign() {
	int i;
	// array is going to keep each text's and choose a new centroid
	float mean_x = 0.0;
	float mean_y = 0.0;
	centroid_x = 0.0;
	centroid_y = 0.0;

	// Sum of the coordinate x -> centroid_x
	// Sum of the coordinate y -> centroid_y 
	for (i = 0; i < rows; i++) {
		centroid_x += array[i][0];
		centroid_y += array[i][1]; 
	}

	mean_x = centroid_x / rows;
	mean_y = centroid_y / rows;
	centroids[closest_centroid][0] = mean_x;
	centroids[closest_centroid][1] = mean_y;

	//for (i = 0; i < M; i++) {
	//	printf("NEW Centroids %d: x = %f  y = %f\n", i + 1, centroids[i][0], centroids[i][1]);
	//} 
}


// Create the clusters here, according to the minimum distance calculation
void createClusters(){
	float distance[M], min_distance;
	int i, c, d, j;

	// Compute the distance of each point with every cluster
	for (d = 0; d < row; d++) {
		for (c = 0; c < M; c++) {
			distance[c] = euclideanDistance(centroids[c][0], centroids[c][1], data[d][0], data[d][1]);
			//printf("Distance from centroid %d = %f\n", c + 1, distance[c]);	
		}
		
		// Find the minimum distance of all the distances for the same point
		min_distance = distance[0];
		for (c = 0; c < M; c++) {
			if (min_distance < distance[c]){
				min_distance = min_distance;
			}
			else {
				min_distance = distance[c];
				closest_centroid = c;
			}
		}

		//printf("min_distance = %f\n", min_distance);
		//printf("Closest centroid = %d\n", closest_centroid + 1);
		//printf("Point selected: x = %f  y = %f\n\n", data[d][0], data[d][1]);

		// Write the points with min distance of each cluster to a file
	   	sprintf(filename, "Cluster%d.txt", closest_centroid + 1);
    	fp[closest_centroid + 1] = fopen(filename, "a");
    	fprintf(fp[closest_centroid + 1], "%f, %f\n", data[d][0], data[d][1]);
    	fclose(fp[closest_centroid + 1]);

		// Remove the point from data[][] to continue with the rest ones 
		/*row = row - 1;
		for (d = 0; d < row; d++) {
			data[d][0] = data[d + 1][0];
			data[d][1] = data[d + 1][1];
		}*/

		//d = -1;		// to loop next the 1st element of data[][]
					// else data[][] will not be fully edited

		// Auxiliary array[][] for keeping the contents of the M-cluster files
		arrayCreation(filename);
		/*for (i = 0; i < rows; i++) {
			printf("ARRAY: x = %f,  y = %f\n", array[i][0], array[i][1]);
		}*/

		// Assign new centroids according to the already created clusters
		newCentroidAssign();

		// array[][]'s lines should be free here
		rows = 1; 
	} 
} 


// Create new M-cluster text files with the correct coordinates (final file)
void exportNewFiles() {
	int i;
	char *s;
	char copy[30];
	char buffer[20];	
	char token[] = ",";
	float original_centroid[1][2];

	// Add the originally initialized cetroids to the re-created M-cluster files 
	for (i = 0; i < M; i++) {
		sprintf(filename, "Cluster%d.txt", i + 1);
    	fp[i + 1] = fopen(filename, "r");
		fgets(copy, 30, fp[i + 1]);

		// Create a new empty file and write the original first centroid 
    	fp[i + 1] = fopen(filename, "w");
    	fprintf(fp[i + 1], "%s\n", copy);	
    	fclose(fp[i+1]);
	}
}


// Algorithm's convocation check
void newClusterAssign() { 
	int i;
	int same = 0;

	// Keep the values of the last centroids to an array and check if the values are 
	// the same with the values of the new centroids after +1 loop of the algorithm to stop
	do {
		for (i = 0; i < M; i++) {
			newCentroids[i][0] = centroids[i][0];
			newCentroids[i][1] = centroids[i][1]; 
		}

		//ReadData(testFile);
		exportNewFiles();
		createClusters();

		for (i = 0; i < M; i++) {
			if ( (centroids[i][0] == newCentroids[i][0]) && (centroids[i][1] = newCentroids[i][1]) ) {
				same = 1;
			}
		}
	} while (same == 0); 
} 


// Error calculation method
float errorCalculation() {
	char buffer[50], *s;
	char token[] = ",";
	int i, j = 0, flag = 0;
	float error = 0.0;
	float final_centroid[2];
	float point[2];
	float distance = 0.0;

	for (i = 0; i < M; i++) {
		final_centroid[0] = centroids[i][0];
		final_centroid[1] = centroids[i][1];

		sprintf(filename, "Cluster%d.txt", i + 1);
    	fp[i + 1] = fopen(filename, "r");
    	
		while ( (fscanf(fp[i + 1], "%s", buffer)) != EOF) {
			// s seperates the comma and gets the number only
			s = strtok(buffer, token);
			if (s != NULL){
				point[j] = atof(s);
				
				if (j == 0) {
					j++;
				}
				else {
					j--;
					flag = 1;
				}
			}

			if (flag == 1) {
				distance = euclideanDistance(final_centroid[0], final_centroid[1], point[0], point[1]);
				flag = 0;
			}
			error += distance;
		}

    	fclose(fp[i+1]);
	}

	return error;
}


int main () {
	float error;
	int i;
	FILE *fp;

	ReadData(testFile);
	defineCentroids();
	createClusters();
	newClusterAssign();

	error = errorCalculation();

	fp = fopen("Centroids.txt", "w");
	for (i = 0; i < M; i++){
		fprintf(fp, "%f, %f\n", centroids[i][0], centroids[i][1]);
	}

	fclose(fp);

	printf("\nInfo with the final centroids is written in 'Centroids.txt' file.\n");

	for (i = 0; i < M; i++) {
		printf("\nCluster %d info is written in 'Cluster%d.txt' file.\n", i + 1, i + 1);
	}

	printf("\nError = %f\n", error);

	/*
	// Free the dynamically created data[][] and array[][]
	for (i = 0; i < rows; i++) {
		free(array[i]);
	}

	for(i = 0; i < row; i++) {
		free(data[i]);
	}
	*/
	return 0;
}
