package inferEngine;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.jdom2.xpath.XPath;

import engineUtility.InvokeMethod;

import constant.Constant;

public class ModelSelect {

	private  String modelSelectRulePath = Constant.model_ruleml_path;
	
	private  HashSet<String>  suitable_set = new HashSet<String>();
    private  HashSet<String> climate_set = new HashSet<String>();
    private  HashSet<String> surface_set = new HashSet<String>();
    private  HashSet<String> output_set = new HashSet<String>();
    private  HashSet<String> input_set = new HashSet<String>();
    private  HashSet<String> time_scale_step_set = new HashSet<String>();
    private  HashSet<String> spatial_scale_discretization_set = new HashSet<String>();
    
    private ModelingScenario modelSc = null;
    
    private List<RuleML> rules = null;
    
    public void initializeScenario(){
    	
    	modelSc = new ModelingScenario();
    	
    	modelSc.con_climate_names.add("climate");
    	modelSc.con_climate_values.add("humid");
    	
    	modelSc.con_surface_names.add("underlying surface type");
    	modelSc.con_surface_values.add("rural");
    	
    	modelSc.con_output_names.add("model outputs");
    	//modelSc.con_output_values.add("runoff,ground water level");
    	modelSc.con_output_values.add("runoff");
    	
    	//modelSc.con_input_names.add("model inputs");
    	//modelSc.con_input_values.add("runoff,mean rain,mean pet,watershed area");
    	
    	modelSc.con_time_scale_step_names.add("time scale");
    	modelSc.con_time_scale_step_values.add("continous");
    	modelSc.con_time_scale_step_names.add("time step");
    	//modelSc.con_time_scale_step_values.add("day");
    	modelSc.con_time_scale_step_values.add("month");
    	
    	modelSc.con_spatial_scale_discretization_names.add("spatial scale");
    	modelSc.con_spatial_scale_discretization_values.add("medium");
    	modelSc.con_spatial_scale_discretization_names.add("spatial discretization");
    	//modelSc.con_spatial_scale_discretization_values.add("semi-distributed");
    	modelSc.con_spatial_scale_discretization_values.add("lumped");
    }
    
    public  void selectModel(){
    	
    	int pattern = 0;
    	
    	if(modelSc.con_input_names.size()>0){
    		
    		this.input_set = this.selectModelByCondition(modelSc.con_input_names,modelSc.con_input_values);
            pattern = 0;
    	}else{
    		
    		pattern = 1;
    	}

    	this.climate_set = this.selectModelByCondition(modelSc.con_climate_names,modelSc.con_climate_values);

    	this.surface_set = this.selectModelByCondition(modelSc.con_surface_names,modelSc.con_surface_values);
    	
    	this.output_set = this.selectModelByCondition(modelSc.con_output_names,modelSc.con_output_values);
    	
    	this.time_scale_step_set = this.selectModelByCondition(modelSc.con_time_scale_step_names,modelSc.con_time_scale_step_values);
    	
    	this.spatial_scale_discretization_set = this.selectModelByCondition(modelSc.con_spatial_scale_discretization_names,
    			modelSc.con_spatial_scale_discretization_values);
    	
    	if(pattern == 0){
    		this.suitable_set = this.input_set;
    		this.suitable_set.retainAll(this.climate_set);
    		this.suitable_set.retainAll(this.surface_set);
        	this.suitable_set.retainAll(this.output_set);
        	this.suitable_set.retainAll(this.time_scale_step_set);
        	this.suitable_set.retainAll(this.spatial_scale_discretization_set);
    	}else{
    		this.suitable_set = this.climate_set;
        	this.suitable_set.retainAll(this.surface_set);
        	this.suitable_set.retainAll(this.output_set);
        	this.suitable_set.retainAll(this.time_scale_step_set);
        	this.suitable_set.retainAll(this.spatial_scale_discretization_set);	
    	}
    	
    	
    	System.out.println(this.suitable_set);
        
    }
    
    private void readRuleML(){
    	
    	this.rules = LoadRuleML.readRuleML(modelSelectRulePath);
    	
    }
    
    private HashSet<String> selectModelByCondition(List<String> names, List<String> values){
    	
    	HashSet<String> models = new HashSet<String>();
    	
    	try{
        	
    		for(RuleML rule : this.rules){
        		
        		boolean flag = false;
        		if(rule.or_and == Or_And.No_Or_And){
        			
        			Atom atom = rule.if_rule.get(0);
        			String var = atom.var;
        			String ind = atom.ind;
        			String rel = atom.rel;
        			
        			String condition_name = names.get(0);
    				String condition_value = values.get(0);
        			
        			if(!var.equals(condition_name)){
        				
        				continue;
        				
        			}else{
        				
        				flag = InvokeMethod.invoke(rel, condition_value, ind);
        			}
        		}else if(rule.or_and == Or_And.And){
        			
        			List<Atom> atoms = rule.if_rule;
        			List<String> vars = new ArrayList<String>();
        			for(Atom atom : atoms){
        				vars.add(atom.var);
        			}
        			
        			///begin 判断两个list是否完全相同
        			if(vars.size()!= names.size()){
						
        				continue;
					}else{
						
						for(String name : names){
							if(!vars.contains(name)){
								continue;
							}
						}
					}
        			/// end 判断两个list是否完全相同
        			
        			int index = 0;
					for(Atom atom: atoms){
						
						String condition_value = values.get(index);
						index++;
						
						String rel = atom.rel;
						String ind = atom.ind;
						
						flag = InvokeMethod.invoke(rel, condition_value, ind);
						if(flag == false){
							break;
						}
						
					}
        		
        		}else{
        			
        			List<Atom> atoms = rule.if_rule;
        			List<String> vars = new ArrayList<String>();
        			for(Atom atom : atoms){
        				vars.add(atom.var);
        			}
        			
        			boolean tag = false;
					for(String name : names){
						if(vars.contains(name)){
							tag = true;
						}
					}
					if(tag == false){
						continue;
					}
					
					int index = 0;
					for(Atom atom: atoms){
						
						String condition_value = values.get(index);
						index++;
						
						String rel = atom.rel;
						String ind = atom.ind;
						
						flag = InvokeMethod.invoke(rel, condition_value, ind);
						if(flag == true){
							break;
						}
						
					}
					
        		}
        		//执行then部分
        		if(flag == true){
        			
        			Atom atom = rule.then_rule.get(0);
        			String ind = atom.ind;
        			
        			if(ind.contains(",")){
						String [] tmps = ind.split(",");
						for(String tmp : tmps){
							models.add(tmp);
						}
					}else{
						
						models.add(ind);
					}
        		}// end of if
        	}// end of for
    	}catch(Exception e){
    		
    		e.printStackTrace();
    	}
    	
    	return models;
    }
    
    public static void main(String[] args){
    	
    	ModelSelect select = new ModelSelect();
    	
    	select.readRuleML();
    	select.initializeScenario();
    	select.selectModel();
    	
    	
    }
    
    
}
