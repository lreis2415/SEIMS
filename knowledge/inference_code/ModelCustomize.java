package inferEngine;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import modelConfig.ModelConfig;

public class ModelCustomize {

	private HashMap<String,String> condition_value = new HashMap<String,String>();
	private List<String> selected_processes = new ArrayList<String>();
	
    public List<String> sorted_componentIDs = new ArrayList<String>();
    public List<String> sorted_algorithmNames = new ArrayList<String>();
    public List<String> sorted_processNames = new ArrayList<String>();
	
	
	private void setScenario(){
		
		condition_value.put("spatial scale", "medium-sized");
		condition_value.put("application purpose", "total runoff simulation");
		condition_value.put("time step", "day");
		condition_value.put("particular process", "snow");
		condition_value.put("underlying surface type", "rural");
		condition_value.put("climate type", "humid");
		condition_value.put("climate input", "TMAX,TMIN,P");
		
	}
	
	private void setScenario(HashMap<String,String> condition_value){
		this.condition_value = condition_value;
	}
	
	private void selectProcess(){
		
		ProcessSelect select = new ProcessSelect();
    	select.readRuleML();
    	select.setScenario(condition_value);
    	selected_processes = select.selectProcess();
	}
	
	private void identifyAlgorithm(){
		 
		  AlgorithmSelect select = new AlgorithmSelect();
		  
		  select.readRuleML();//read algorithms' RuleML
		  select.setAlgorithmName4component(); //build the mapping between algorithm and component  
		  select.setScenario(selected_processes, condition_value);//set selected hydrological processes and modeling scenario for identifying algorithm for each process
		  select.load_all_algorithms();//load all available algorithms for each selected hydrological process
	   	  select.selectAlgorithm();// identify/select one or more algorithms for each process
	   	  select.generateAlgorithmGroup();// generated algorithm groups, each group consists of a set of algorithms 
	   	  
	   	  
	   	  select.check_Component_Input2Output();//check whether the components of each group are compatible
		  ////select.componentSort();//sort each component's order by input-output relationship
		  
		  this.sorted_componentIDs = select.get_sorted_componentIDs();
		  this.sorted_algorithmNames = select.get_sorted_algorithmNames();
		  this.sorted_processNames = select.get_sorted_processNames();
		
	}
	
	private void generate_config(String timestamp){
		
		String[] inputs = condition_value.get("climate input").split(",");
		List<String> timeSeryInputs = Arrays.asList(inputs);
		String purpose = condition_value.get("application purpose");
		
		ModelConfig.writeConfig(timestamp,purpose, timeSeryInputs, this.sorted_processNames, this.sorted_algorithmNames ,  this.sorted_componentIDs);
		
	}
	
	public void customize_model(HashMap<String,String> condition_value, String timestamp){  
		
		 this.setScenario(condition_value);
		 this.selectProcess();
		 this.identifyAlgorithm();
		 this.generate_config(timestamp);
		                        
	}
	
	 public static void main(String[] args){
		 
		 ModelCustomize modelCustomize = new ModelCustomize();
		 modelCustomize.setScenario();
		 modelCustomize.selectProcess();
		 modelCustomize.identifyAlgorithm();
		 modelCustomize.generate_config("");    
	 }
}
