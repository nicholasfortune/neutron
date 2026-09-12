# Binary Architecture
The `.neu` file format is made of a header with crucial parsing information, and blocks with the weights and biases placed in an alternating fasion.

| **Block Number** 	| **Block Purpose**   	| **Data Type**                      	| **Description / Notes**                                                 	|
|------------------	|---------------------	|------------------------------------	|-------------------------------------------------------------------------	|
| N/A              	| Version             	| uint32_t                           	| Version number of the file                                              	|
| N/A              	| Blocks              	| uint32_t                           	| Counts the number of non-header blocks                                  	|
| N/A              	| Block sizes         	| "Blocks" number of uint32_t's      	| One uint32_t for every block, counts the amount of floats in that block 	|
| N/A              	| Config size         	| uint32_t                           	| Counts the number of config entries                                     	|
| N/A              	| Config data entries 	| "Config size" number of uint32_t's 	| One uint32_t for every config entry                                     	|
| 0                	| Weights of layer 1  	| One float for each weight   			| Holds any amount of weights necessary                                   	|
| 1                	| Biases of layer 1   	| One float for each bias     			| Holds any amount of biases necessary                                    	|
| 2                	| Weights of layer 2  	| One float for each weight   			| Holds any amount of weights necessary                                   	|
| 3                	| Biases of layer 2   	| One float for each bias     			| Holds any amount of biases necessary                                    	|


Weights are on every even block number, and biases are on every odd block number


# Config Data Format History 
### v1
| **Config Number** 	| **Config Purpose** 	|
|-------------------	|--------------------	|
| 0                 	| Input size         	|