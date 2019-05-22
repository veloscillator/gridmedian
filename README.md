# Grid Median
Author: Ken Kochis

## Specification

Given a video file and dimensions (e.g. 32x32, 64x64, 128x128), 
extract keyframes from the video, convert the frame into grayscale, 
split each frame into a grid of said dimensions, calculate median 
value of all the pixels of each cell of the grid and write the values to a CSV file together with the timestamp of the frame.

Example:

If a frame timestamp (in seconds) is 3.14 and the dimensions are 3x3 an example line might look like:

  3.14,42,255,9,13,67,0,27,33,123  // timestamp + 9 values (3x3) 

Answer should be posted in a git repo.

## Building and Running
- ```make```
- Run ```gridmedian``` for usage

To run unit tests:
 - ```make test```

Special flags:
- The DEBUG_PRINT_FRAMES compiler flag can be used to print keyframes of a video to disk for testing purposes.


 ## Dependencies

- ffmpeg version 4
- libavcodec, libavformat, libavutil, libswscale (with development headers)
- g++


## Implementation Details

Video decoding/keyframe extraction code is in video.h/cc. The VideoDecoder class allows iteration of keyframes
and provides a Frame type with a matrix-like interface. grid.h contains code for splitting a frame into a grid
and calculating median.

unittest_grid.h contains unit tests for the methods in grid.h. sample.mp4 is a small video whose expected output
is in sample_output.csv.


### Assumptions
- I'm assuming median values should be integers because floating point values don't make sense for this type of data
(and due to the example provided).
- Grid width/height <= frame width/height.


### Decoding Video Frames

Decoding video frames involves straightforward use of libavformat/libavcodec. This is encapsulated by the VideoDecoder class. This
class is structured to allow consumers to iterate through frames/keyframes and view grayscale contents.

FUTURE WORK: Due to time constraints, this function assumes video format is YUV420P (see TODOs for more info).


### Median Calculation

Median is defined as the "middle" element in a sorted list of data points. If the number of data points is even, it is defined as
the mean of the two middle elements (see https://en.wikipedia.org/wiki/Median).

Here, we calculate median by first sorting the data
points and then selecting the middle element. The runtime complexity of this algorithm is O(nlgn) (given by the complexity of the
sorting algorithm) while the space complexity is O(n) where n = number of elements in a cell. There may exist median algorithms that
give better runtime/space complexity. Given that cells aren't expected to be too large, fancier algorithms likely won't result in speed improvements
that justify their increase in code complexity.


### Grid Calcualtion

Consider each frame as a matrix of pixels. Our goal is to split each frame into a grid of gridWidth x gridHeight cells such that each
cell contains a roughly equal number of pixels. Given that we have ```gridWidth <= frameWidth``` and ```gridHeight <= frameHeight```, it is
always possible to create exactly gridWidth x gridHeight cells. However, if gridWidth x gridHeight does not evenly divide into frameWidth x frameHeight,
the cells will contain an uneven number of pixels. The algorithm for dividing a matrix of pixels into cells is as follows:

#### Case 1: gridWidth x gridHeight evenly divide into frameWidth x frameHeight
If gridWidth x gridHeight evenly divide into frameWidth x frameHeight, each cell will be of size ```(frameWidth / gridWidth) x (frameHeight / gridHeight)```.
For example, if the frame is 6x4 and our grid is 2x2, we split into cells as follows (A, B, C, and D are the separate cells. Each letter denotes a pixel):

```
 A A A B B B
 A A A B B B
 C C C D D D
 C C C D D D
```

#### Case 2: gridWidth x gridHeight do not evenly divide into frameWidth x frameHeight
If gridWidth x gridHeight does not divide evenly into frameWidth x frameHeight, we can calculate an optimal solution resulting in the correct number
of cells as follows:

Consider rows and columns independently. This reduces the problem to dividing an array of size N (e.g. frameWidth) into roughly equal chunks of size M (e.g.
gridWidth) (we simply do this for each of the two dimensions). Note that we have M <= N from above. We can get an optimal solution by combining chunks
of length ```ceil(N / M)``` with chunks of length ```floor(N / M)``` (note that ```ceil(N / M) == 1 + floor(N / M)``` because ```N % M != 0```). The number of each type of chunk is given by:

```
numCeilChunks = N - M * (floor(N / M))
numFloorChunks = M - numCeilChunks
```

For example, let's take an 8 x 7 matrix and split it into a 3 x 5 grid. First, let's split 8 colums in 3 (N = 8, M = 3).
```
ceil(8 / 3) = 3
floor(8 / 3) = 2
numCeilChunks = 8 - 3 * floor(8 / 3) = 2
numFloorChunks = 3 - 2 = 1
```
So, we split our columns into 2 chunks of length 3 (chunks A and B) and one chunk of length 2 (chunk C):
```
A A A B B B C C
```
To split 7 rows into 5 rows:
```
ceil(7 / 5) = 2
floor(7 / 5) = 1
numCeilChunks = 7 - 5 * floor(7 / 5) = 2
numFloorChunks = 5 - 2 = 3
```
So we get:

```
A
A
B
B
C
D
E
```

Putting the two together, we get our combined matrix of 3 x 5 cells:
```
A A A B B B C C
A A A B B B C C
D D D E E E F F
D D D E E E F F
G G G H H H I I
J J J K K K L L
M M M N N N O O
```

### Future Work

- Parallelism: frames can be processed independently. This workload can be parallelized by enqueuing all frames
into a shared queue and spawning multiple threads to process frames (data from the threads would then have to be combined in the right
order). This will likely result in noticeable performance benefit for very large video files, though memory utilization will have to
be taken into account.
- End-to-end testing: for the sake of time (and due to limitations on git repo size) I have not included end-to-end tests or sample
videos in this repo except for sample.mp4 and sample_output2x3.csv.

