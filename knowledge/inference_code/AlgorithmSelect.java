package inferEngine;


import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;


import org.jdom2.Document;
import org.jdom2.Element;
import org.jdom2.Namespace;
import org.jdom2.input.SAXBuilder;
import org.jdom2.xpath.XPath;

import engineUtility.InvokeMethod;

import constant.Constant;

public class AlgorithmSelect {
    
	//知识库路径 knowledge base path
    private  String algorithmSelectBasePath = Constant.algorithm_ruleml_path;
    private  String componentRdfBasePath =  Constant.component_rdf_folder_path;
	private  String algorithmRdfPath = Constant.algorithm_rdf_path;
    
    //算法选择场景 algortihm scenario
    private AlgorithmScenario algorithmSc = new AlgorithmScenario();
    
    //参与水文模拟的子过程名称集合 selected processes
    private List<String> processes = new ArrayList<String>();

    //每个算法名称对应的子过程名称，HashMap <算法名称，过程名称>
    //HashMap <algorithme_name, process_name>
    private HashMap<String,String> all_algorithm_process = new HashMap<String,String>();
    
    //每个子过程名称所对应的推荐算法的名称
    //HashMap <process_name, names of identified algorithms>
    ////private HashMap<String,HashSet<String>> process_identifying_algorithm = new HashMap<String,HashSet<String>>();
    private HashMap<String,List<String>> process_identifying_algorithm = new HashMap<String,List<String>>();
    //规则集合 rule list
    private List<RuleML> rules = null;
    
    //组件rdf集合，仅仅是推荐算法所对应的组件 rdf of components of identified algorithms
    //HashMap<algorithm name, ComponentMeta>
    private HashMap<String, ComponentMeta> componentMetas = new HashMap<String, ComponentMeta>(); 
    
    //组件名称数组，仅仅是所推荐算法对应的组件名称 names of components of identified algorithms
    ////private  List<String> comnames = new ArrayList<String>();
    private  List<List<String>> comnames = new ArrayList<List<String>>();
    
    //现有模型输入 existing model input data
    private List<String> existingData = new ArrayList<String>();
    
    
    //HashMap<String,String><算法名称，组件名称>，针对所有组件// for all components
    //HashMap<algorithm_name,component_name>
    private HashMap<String,String> algorithm_component = new HashMap<String,String>();
    
    
    //HashMap<String,String><组件名称，算法名称>，针对所有组件// for all components
    //HashMap<component_name, algorithm_name>
    private HashMap<String,String> component_algorithm = new HashMap<String,String>();
    
    //HashMap<String,String><算法名称，算法简称>，针对所有算法 // for all algorithms
    //HashMap<algorithm_fullname, algorithm_shortname>
    private HashMap<String,String> algorithm_shortname = new HashMap<String,String>();
    
    
    //排序过后的组件ID IDs of sorted components
    private List<String> sorted_componentIDs = new ArrayList<String>();
    //排序过后的算法名称 names of sorted algorithms
    private List<String> sorted_algorithmNames = new ArrayList<String>();
    //排序过后的过程名称 names of sorted processes
    private List<String> sorted_processNames = new ArrayList<String>();
    
    //加载子过程集合中每个子过程对应的算法
    //load algorithms for each process
    public void load_all_algorithms(){
    	
	    	SAXBuilder sb = new SAXBuilder();
			Document filesdoc = null;
			try{
				
				filesdoc = sb.build(algorithmRdfPath);
				XPath xpath = XPath.newInstance("/rdf:RDF/rdf:Description");
				Namespace rdf = Namespace.getNamespace("rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#");
				Namespace hydro = Namespace.getNamespace("hydro","http://www.cas.ac.cn/hydrology#");
				xpath.addNamespace(rdf); 
			    xpath.addNamespace(hydro); 
			
				List<Element> _decriptions = (List<Element>) xpath.selectNodes(filesdoc);
				//
				for(Element _decription : _decriptions){
					
				    String algorithm_name = _decription.getAttributeValue("about");
					String shortfor_name = _decription.getChild("shortfor", hydro).getText();
					String process_name = _decription.getChild("process", hydro).getText();
					//System.out.println(algorithm_name + ":" + process_name);        
					all_algorithm_process.put(algorithm_name, process_name);
					algorithm_shortname.put(algorithm_name, shortfor_name);
				}//end of for
			
			}catch(Exception e){
				
				e.printStackTrace();
			}

    }
    
    //初始化场景  initialization scenario
    private void initializeScenario(){
    	/*
    	this.processes.add("Interception");
    	this.processes.add("PET");
    	this.processes.add("Infiltration");
    	this.processes.add("Ground Water");
    	this.processes.add("Channel Flow");
    	this.processes.add("Overland Flow");
    	this.processes.add("Snow Melt");
    	this.processes.add("Soil Temperature");
    	this.processes.add("Depression");
    	this.processes.add("Percolation");
    	this.processes.add("Soil Evaporation");
    	this.processes.add("Subsurface Runoff");
    	this.processes.add("Water Balance");
    	this.processes.add("Sediment Yield");
    	*/ 
    	this.processes.add("Interception");
    	this.processes.add("Percolation");
    	this.processes.add("Ground Water");
    	this.processes.add("Infiltration");
    	this.processes.add("Depression");
    	this.processes.add("Overland Flow");
    	this.processes.add("Interflow Routing");
    	this.processes.add("Channel Flow");
    	this.processes.add("Splash Erosion");
    	this.processes.add("Overland Erosion");
    	this.processes.add("Channel Erosion");
    	
    	algorithmSc.condition_value.put("climate input", "TMAX,TMIN,P");
    	algorithmSc.condition_value.put("time step", "hour");
    	
    	this.existingData.add("TMAX");
    	this.existingData.add("TMIN");
    	this.existingData.add("P");
    }
    // set scenario
    public void setScenario(List<String> selectedProcess, HashMap<String,String> condition_value){
    	
    	for(String process : selectedProcess){
    		this.processes.add(process);
    	}
    	Iterator iter = condition_value.entrySet().iterator();
    	while (iter.hasNext()) {
    	    Map.Entry entry = (Map.Entry) iter.next();
    	    Object condition = entry.getKey();
            Object value = entry.getValue();
            algorithmSc.condition_value.put(condition.toString(), value.toString());
    	}
    	List<String>input_keys = new ArrayList<String>();
    	input_keys.add("climate input");
    	input_keys.add("hydrology input");
    	input_keys.add("underlying surface input");
    	for(String input_key : input_keys){
    		if(algorithmSc.condition_value.containsKey(input_key)){
        		
        		String climate_input = algorithmSc.condition_value.get(input_key);
        		String[] climate_inputs = climate_input.split(",");
        		for(String _input : climate_inputs){
        			this.existingData.add(_input);
        		}
        	}
    	}
    	
    }
    //generate Algorithm Group
    public void generateAlgorithmGroup(){
    	
    	List<List<String>>  models = this.enumerate(this.processes.size() -1);
    	//System.out.println(models);
    	int len = models.size();
    	for(int i=0; i<len; i++){
    		this.comnames.add(new ArrayList<String>()); 
    	}
    	for(int i =0; i<len; i++){
    		List<String> _algorithms = models.get(i);
    		System.out.println("the selected algorithms for each process are as follow:");
    		for(String _algorithm : _algorithms){
    			 System.out.println(_algorithm);
    			 System.out.println(this.algorithm_component.get(_algorithm));
    			 this.comnames.get(i).add(this.algorithm_component.get(_algorithm));
    		}
    		System.out.println("----------------------------------------------------");
    	} 	
    }
    // select algorithm
    public  void selectAlgorithm(){
     
    	this.selectAlgorithmByCondition();  
    	System.out.println(this.process_identifying_algorithm.values());
    }
    // load rules for algorithms              
    public void readRuleML(){
    	
    	this.rules = LoadRuleML.readRuleML(algorithmSelectBasePath);
    }
    
    // load rdf of components
    private void loadComInfo(List<String> components){
    	
		try{ 
		
			for(String component : components){
		    	
				ComponentMeta info = new ComponentMeta();
				info.componentName = component;
				//System.out.println("component name:" + component);
				SAXBuilder sb = new SAXBuilder();
				Document filesdoc =  sb.build(componentRdfBasePath + component + ".xml");
	    		
				XPath xpath = XPath.newInstance("/rdf:RDF/rdf:Description/hydro:hasInputs/rdf:Bag/rdf:li");
	    		xpath.addNamespace("rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#"); 
			    xpath.addNamespace("hydro","http://www.cas.ac.cn/hydrology#"); 
			    
			    List<Element> _inputs = (List<Element>)xpath.selectNodes(filesdoc);
			    for(Element _input:_inputs){
			    	String inputname = _input.getText();
			        info.inputNames.add(inputname);
			    }
			    
			    xpath = XPath.newInstance("/rdf:RDF/rdf:Description/hydro:hasOutputs/rdf:Bag/rdf:li");
	    		xpath.addNamespace("rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#"); 
			    xpath.addNamespace("hydro","http://www.cas.ac.cn/hydrology#"); 
				
			    List<Element> _outputs = (List<Element>)xpath.selectNodes(filesdoc);
			    for(Element _output:_outputs){
			    	String outputname = _output.getText();
			    	info.outputNames.add(outputname);
			    }
			    
			    xpath = XPath.newInstance("/rdf:RDF/rdf:Description/hydro:hasOptionalInputs/rdf:Bag/rdf:li");
	    		xpath.addNamespace("rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#"); 
			    xpath.addNamespace("hydro","http://www.cas.ac.cn/hydrology#"); 
				
			    List<Element> _optinalinputs = (List<Element>)xpath.selectNodes(filesdoc);
			    for(Element _opinput:_optinalinputs){
			    	String opinputname = _opinput.getText();
			    	info.optionalInputNames.add(opinputname);
			    }
			    
			    
			    componentMetas.put(component, info);
			}
			
				
		}catch(Exception e){
				
			e.printStackTrace();
		}
			

    }
    
    // 
	public boolean checkInput2Output(List<String> com_names){
		
		boolean flag = false;
		for(String com_name : com_names){
			
			if(!com_name.equals("TSD_RD")&&!com_name.equals("ITP")){
				
				ComponentMeta algorithMeta = componentMetas.get(com_name);
				HashSet<String> inputNames = algorithMeta.inputNames;
				flag = false;
				for(String inputName : inputNames){
					
					flag = this.checkInput(inputName,com_names);
					if(flag == false){
						System.out.println("com_name:" + com_name);
						System.out.println("inputName:" + inputName);
						break;
					}
						
				}
				if(flag == false){
					
					break;
				}
			}
			
		}
		
		//System.out.println("flag:" + flag);
		if(flag == true){
			System.out.println("the components of slected alogrithms are compatible!");
			System.out.println("-----------------------------------------------------");
		}
		
		return flag;
	}
    
   private boolean checkInput(String inputName,List<String> com_names){
		
		boolean flag = false;
		
		if(this.existingData.contains(inputName)){
			
			return true;
		}
		
		for(String _com_name : com_names){
			ComponentMeta _algorithMeta = componentMetas.get(_com_name);
			
			HashSet<String> outputNames = _algorithMeta.outputNames;
			if(outputNames.contains(inputName)){
				flag = true;
				break;
			}
		}

		return flag;
	}
	
   private void selectAlgorithmByCondition(){
    	
        try{
        	
    		for(RuleML rule : this.rules){
        		
        		boolean flag = false;
        		if(rule.or_and == Or_And.No_Or_And){
        			
        			Atom atom = rule.if_rule.get(0);
        			String var = atom.var;
        			String ind = atom.ind;
        			String rel = atom.rel;
        			
    				String condition_value = algorithmSc.condition_value.get(var);
        			
                    if(condition_value == null){
						
						flag =false;
					}else{
						 flag = InvokeMethod.invoke(rel, condition_value, ind);
		        			
					}
    				
        		}else if(rule.or_and == Or_And.And){
        			
        			List<Atom> atoms = rule.if_rule;
        			for(Atom atom : atoms){
        				
        				String var = atom.var;
        				String rel = atom.rel;
        				String ind = atom.ind;
        				
        				String condition_value = algorithmSc.condition_value.get(var);
						if(condition_value == null){
							
							flag = false;
							break;
							
						}else{
							flag = InvokeMethod.invoke(rel, condition_value, ind);
							if(flag == false){
								break;
							}
						}
        			}//end of for
        			
        		}else if(rule.or_and == Or_And.Or){
        			
        			List<Atom> atoms = rule.if_rule;
        			for(Atom atom : atoms){
        				
        				String var = atom.var;
        				String rel = atom.rel;
        				String ind = atom.ind;
        				String condition_value = algorithmSc.condition_value.get(var);
                        if(condition_value == null){
							
							flag = false;
							continue;
							
						}else{
							
							flag = InvokeMethod.invoke(rel, condition_value, ind);
							if(flag == true){
								break;
							}
						}
        			}//end of for
					
        		}
        		//
        		if(flag == true){
        			
        			Atom atom = rule.then_rule.get(0);
        			String ind = atom.ind;
        			String rel = atom.rel;
        			if(rel.replace(" ", "").equals("isselected")){
        				String process_name = all_algorithm_process.get(ind);
        				
        				if(this.processes.contains(process_name)){

        					if(process_identifying_algorithm.containsKey(process_name)){
        						                                        
        						process_identifying_algorithm.get(process_name).add(ind);
 
        					}else{
        						////HashSet<String> algos = new HashSet<String>();
        						List<String> algos = new ArrayList<String>();
        						algos.add(ind);
        						process_identifying_algorithm.put(process_name, algos);
        					}	               
        				}
        				
        			}
        		}// end of if
        	}// end of for
    	}catch(Exception e){
    		
    		e.printStackTrace();
    	}
    }
    
	private List<List<String>> enumerate(int n){
		
		List<List<String>> _temps = new ArrayList<List<String>>();
		if(n == 0){
			//System.out.println("n:" + n);
			//System.out.println(this.processes.get(0));
			//System.out.println(this.process_identifying_algorithm.get(this.processes.get(0)));
			Object[] tem = this.process_identifying_algorithm.get(this.processes.get(0)).toArray();
			for(Object te : tem){
				List<String> nums = new ArrayList<String>();
				nums.add((String) te);
				_temps.add(nums);
			}
			
			return _temps;
			
		}else{
			//System.out.println("n:" + n);
			List<List<String>> _arrays = enumerate( n -1);
			Object[] array =  this.process_identifying_algorithm.get(this.processes.get(n)).toArray();
			int len = array.length;
			
			for(int i =0;i < len ; i++){
				
				for(List<String> _array : _arrays){
					List<String> nums = new ArrayList<String>();
					nums.add((String) array[i]);
					
				    nums.addAll(_array);
				    _temps.add(nums);
				
				}
			}
		}
		
		return _temps;
   }
   
   private List<String> FindNextAlgs(String comname, List<String> comnames){
	   
	   
	   List<String> nextalgnames = new ArrayList<String>();
	   HashSet<String> outputs = this.componentMetas.get(comname).outputNames;
	   
	   for(String output : outputs){
		   for(String name : comnames){
			   HashSet<String> inputs = this.componentMetas.get(name).inputNames;
			   HashSet<String> opinputs = this.componentMetas.get(name).optionalInputNames;
			   
			
			   if(inputs.contains(output) || opinputs.contains(output)){
				   if(!nextalgnames.contains(name))
				   {
					   nextalgnames.add(name); 
					   
				   }
			   }
		   }
	   }
	   
	   
	   return nextalgnames;
   }	

   
   //读取每个组件所对应的算法名称  Read the algorithm name of each component
   public void setAlgorithmName4component(){
	   
	    List<String> components = new ArrayList<String>();
	    File file = new File(componentRdfBasePath);
	    String[] file_names =  file.list();
	     try{ 
			
			for(String file_name : file_names){
		    	
				SAXBuilder sb = new SAXBuilder();
				Document filesdoc =  sb.build(componentRdfBasePath + file_name);
	    		
				XPath xpath = XPath.newInstance("/rdf:RDF/rdf:Description/hydro:algorithm");
	    		xpath.addNamespace("rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#"); 
			    xpath.addNamespace("hydro","http://www.cas.ac.cn/hydrology#"); 
			    
			    List<Element> _algorithm = (List<Element>)xpath.selectNodes(filesdoc);
			    String algorithm_name = _algorithm.get(0).getText();
			   
			    xpath = XPath.newInstance("/rdf:RDF/rdf:Description");
	    		xpath.addNamespace("rdf","http://www.w3.org/1999/02/22-rdf-syntax-ns#"); 
	    		List<Element> _description = (List<Element>)xpath.selectNodes(filesdoc);
	    		String component_name = _description.get(0).getAttribute("about").getValue();
	    		
	    		//System.out.println(algorithm_name + ":" + component_name);
	    		this.algorithm_component.put(algorithm_name,component_name);
	    		
	    		this.component_algorithm.put(component_name,algorithm_name);
			}
			
				
		 }catch(Exception e){
				
			e.printStackTrace();
		}
	   
   }
   
   public void check_Component_Input2Output(){
	   
	   ////this.loadComInfo(this.comnames);
	   ////this.checkInput2Output(this.comnames);
	   //this.loadComInfo(this.comnames.get(0));
	   //this.checkInput2Output(this.comnames.get(0));
	   int len = this.comnames.size();
	   int index = -1;
	   for(int i =0; i<len; i++){
		   this.loadComInfo(this.comnames.get(i));
		   boolean flag = this.checkInput2Output(this.comnames.get(i));
		   if(flag){
			  index = index + 1;
		   }
	   }
	   if(index>=0){
		   this.componentSort(index);//sort each component's order by input-output relationship
	   }
   }
   
   public void componentSort(int index){
	   
	    Hashtable<Integer,String> table = new Hashtable<Integer,String>();
		
		Hashtable<String,Integer> table_number = new Hashtable<String, Integer>();
		
		
		int len =  this.comnames.get(index).size();
		
		for(int i =0; i< len; i++){
			
			table.put(i, comnames.get(index).get(i));       
			
			table_number.put(comnames.get(index).get(i), i);
		}
		
		Graph graph = new Graph(len);     
		
		for(String comname : comnames.get(index)){ 
			
			List<String> nextalgnames = this.FindNextAlgs(comname,comnames.get(index));
			
			if(comname.contains("DEP_LINSLEY")){
				
				if(nextalgnames.contains("SUR_MR")){
					nextalgnames.remove("SUR_MR");
					
				}
				if(nextalgnames.contains("SUR_CN")){
					nextalgnames.remove("SUR_CN");
					
				}
				
			}
			
           if(comname.contains("DEP_FS")){
				
				if(nextalgnames.contains("SUR_SGA")){
					nextalgnames.remove("SUR_SGA");
					
				}
				
			}
          
			int algnumber = (Integer) table_number.get(comname);
			
			int le = nextalgnames.size();
			
			for(int i =0 ;i < le; i++){
				
				int nextalgnumber = (Integer)  table_number.get(nextalgnames.get(i));
				
				graph.addEdge(algnumber, nextalgnumber);
				
				//System.out.println(table.get(algnumber) + "---->" + table.get(nextalgnumber));
			}
			
		}
	   	
       graph.TopSort();     
       int[] list = graph.getResult();     
       //System.out.println("sort order is: ");
       System.out.println("the components executing order in SEIMS is: ");
       for(int i : list){                  
           System.out.println(table.get(i));  
           this.sorted_componentIDs.add(table.get(i));
       }
       System.out.println("--------------------------------------------");
   }	

   public List<String> get_sorted_componentIDs(){
	 
	   //System.out.println(this.sorted_componentIDs);
	   return this.sorted_componentIDs;
   };
   
   public List<String> get_sorted_algorithmNames(){
	   
	   for(String com: this.sorted_componentIDs){
		   this.sorted_algorithmNames.add(this.component_algorithm.get(com));
	   }
	   //System.out.println(this.sorted_algorithmNames);
	   return  this.sorted_algorithmNames;
   }
   
   public List<String> get_sorted_processNames(){
	   
	   for(String alg: this.sorted_algorithmNames){
		   this.sorted_processNames.add(this.all_algorithm_process.get(alg));
	   }
	   //System.out.println(this.sorted_processNames);
	   return  this.sorted_processNames;
   }
   
   public static void main(String[] args){
   	
	 
	  AlgorithmSelect select = new AlgorithmSelect();
	  
	  select.readRuleML();//read algorithms' RuleML
	  select.setAlgorithmName4component(); //build the mapping between algorithm and component  
	  select.initializeScenario();//initialize selected hydrological processes and modeling scenario for identifying algorithm for each process
	  select.load_all_algorithms();//load all available algorithms for each selected hydrological process
   	  select.selectAlgorithm();// identify/select one or more algorithms for each process
   	  select.generateAlgorithmGroup();// generated algorithm groups, each group consists of a set of algorithms 
   	  
   	  
   	  select.check_Component_Input2Output();//check whether the components of each group are compatible
	  ////select.componentSort();//sort each component's order by input-output relationship
	  
	  select.get_sorted_componentIDs();
	  select.get_sorted_algorithmNames();
	  select.get_sorted_processNames();
	  
	    
   }
}
