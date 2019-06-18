package inferEngine;

import java.util.ArrayList;
import java.util.Stack;

public class Graph {   
	  
    int vertexNum;   //the number of vertex
    ArrayList<ArrayList<Integer>>  table; //table is the adjacency list, table.get(i) store the vertexs which are the next ones of the i-th vertex  
    Stack<Integer> stack;  //store the vertexs whose in-degree is 0 
    int[] result;   //the rusult of TopSort
    int[] in;// in-degree,in[i] is the in-degree of the i-th vertex     
  
    /**  
     *   
     * create a graph 
     *   
     * @param num  
     * the number of vertex 
     *   
     */  
    public Graph(int num) {   
  
        vertexNum = num;   
        table = new ArrayList<ArrayList<Integer>> (vertexNum);   
        for(int i=0;i<vertexNum;i++){ 
        	table.add(new ArrayList<Integer>());
        }
        stack = new Stack<Integer>();   
        result = new int[vertexNum];   
        in = new int[vertexNum];   
  
    }   
  
    /**  
     * add edge: I----->J
     *   
     * @param I  
     *         
     * @param J  
     *         
     * @return 
     */  
    public boolean addEdge(int I, int J) {   
  
        if (J == I) {   
            return false;   
        }   
        
        if (I < vertexNum && J < vertexNum && I >= 0 && J >= 0) {    
            if (isEdgeExists(I, J)) {    
                return false;   
            }   
            table.get(I).add(J);   
            in[J]++;   
            return true;   
        }   
        return false;   
    }   
  
 
    public boolean isEdgeExists(int i, int j) {   
  
        if (i < vertexNum && j < vertexNum && i >= 0 && j >= 0) {   
  
            if (i == j) {   
                return false;   
            }   
  
            if (table.get(i) == null) {   
               return false;
            }   
  
            for (int q = 0; q < table.get(i).size(); q++) {   
                if (((Integer) table.get(i).get(q)).intValue() == j) {    
                    return true;     
                }   
            }   
        }   
        return false;   
    }   
  
    public void TopSort() {   
  
        for (int i = 0; i < vertexNum; i++)
            if (in[i] == 0)   
                stack.push(i);   
        int k = 0;   
        while (!stack.isEmpty()) {
            result[k] = (Integer) stack.pop();
            if (table.get(result[k]) != null) { 
                for (int j = 0; j < table.get(result[k]).size(); j++) {   
                    int temp = (Integer) table.get(result[k]).get(j);
                    if (--in[temp] == 0) {
                        stack.push(temp);   
                    }     
                }    
            }   
            k++;     
        }   
  
        if (k < vertexNum) {   
            System.exit(0);    
        }     
    }   
  
    public int[] getResult() {   
        return result;     
    }   
  
}  

