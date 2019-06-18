package inferEngine;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import org.jdom2.xpath.XPath;

import engineUtility.InvokeMethod;

import constant.Constant;

public class ProcessSelect {

    
	
	private  String processSelectBasePath =   Constant.process_ruleml_path;
	
    private ProcessScenario processSc = new ProcessScenario();;
    
    private List<String> processes = new ArrayList<String>();
    
    private List<RuleML> rules = null;
    
    private void initializeScenario(){
    	
    	
    	
    	processSc.condition_value.put("spatial scale", "medium-sized");
    	processSc.condition_value.put("application purpose", "total runoff simulation");
    	processSc.condition_value.put("time step", "day");
    	processSc.condition_value.put("particular process", "snow");
    	processSc.condition_value.put("underlying surface type", "rural");
    	processSc.condition_value.put("climate type", "humid");
    	
    }
    
    public void setScenario(HashMap<String,String> condition_value){
    	
    	Iterator iter = condition_value.entrySet().iterator();
    	while (iter.hasNext()) {
    	    Map.Entry entry = (Map.Entry) iter.next();
    	    Object condition = entry.getKey();
            Object value = entry.getValue();
            processSc.condition_value.put(condition.toString(),value.toString());
    	}
    }
    
    public  List<String>  selectProcess(){
    	
    	
    	this.selectProcessByCondition();
    	//System.out.println(processes);
    	System.out.println("the selected processes are as follow:");
    	for(String process : processes){
    		
    		System.out.println(process);
    	}
    	System.out.println("----------------------------------------");
    	return processes;
        
    }
    
    public void readRuleML(){
    	
    	this.rules = LoadRuleML.readRuleML(processSelectBasePath);
    	
    }
    
    private void selectProcessByCondition(){
    	
        try{
        	
    		for(RuleML rule : this.rules){
        		
        		boolean flag = false;
        		if(rule.or_and == Or_And.No_Or_And){
        			
        			Atom atom = rule.if_rule.get(0);
        			String var = atom.var;
        			String ind = atom.ind;
        			String rel = atom.rel;
        			
    				String condition_value = processSc.condition_value.get(var);
        			
        		    flag = InvokeMethod.invoke(rel, condition_value, ind);
        			
        		}else if(rule.or_and == Or_And.And){
        			
        			List<Atom> atoms = rule.if_rule;
        			for(Atom atom : atoms){
        				
        				String var = atom.var;
        				String rel = atom.rel;
        				String ind = atom.ind;
        				
        				String condition_value = processSc.condition_value.get(var);
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
        				String condition_value = processSc.condition_value.get(var);
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
        			
        			if(ind.contains(",")){
						String [] tmps = ind.split(",");
						for(String tmp : tmps){
							if(!this.processes.contains(tmp)){
								this.processes.add(tmp);	
							}
						}
					}else{
						if(!this.processes.contains(ind)){
							this.processes.add(ind);	
						}
						
					}
        		}// end of if
        	}// end of for
    	}catch(Exception e){
    		
    		e.printStackTrace();
    	}
    }
    
    public static void main(String[] args){
    	
    	ProcessSelect select = new ProcessSelect();
    	select.readRuleML();
    	select.initializeScenario();
    	select.selectProcess();
    }
}
