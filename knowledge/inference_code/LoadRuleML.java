package inferEngine;

import java.util.ArrayList;
import java.util.List;

import org.jdom2.Document;
import org.jdom2.Element;
import org.jdom2.Namespace;
import org.jdom2.input.SAXBuilder;
import org.jdom2.xpath.XPath;

public class LoadRuleML {
	
	   public static List<RuleML>  readRuleML(String xmlPath){
	    	
		    List<RuleML> rules = new ArrayList<RuleML>();
	    	SAXBuilder sb = new SAXBuilder();
			Document filesdoc = null;
			try{
				
				filesdoc = sb.build(xmlPath);
				XPath xpath = XPath.newInstance("/ruleml:RuleML/ruleml:Assert/ruleml:Implies");	
				Namespace ruleml = Namespace.getNamespace("ruleml","http://ruleml.org/spec");
				xpath.addNamespace(ruleml);
				List<Element> _rules = (List<Element>) xpath.selectNodes(filesdoc);
				//
				for(Element _rule : _rules){
					
					
					RuleML rule = new RuleML();
					
					Element _if = _rule.getChild("if", ruleml);
					Element _And_if = _if.getChild("And", ruleml);
					Element _Or_if = _if.getChild("Or", ruleml); 
					
					if(_And_if == null && _Or_if == null){
					    
						Element _Atom_if = _if.getChild("Atom", ruleml);
						
						Element _Var_if = _Atom_if.getChild("Var", ruleml);
						Element _Ind_if = _Atom_if.getChild("Ind", ruleml);
						Element _op_if = _Atom_if.getChild("op", ruleml);
						Element _Rel_if = _op_if.getChild("Rel", ruleml);
						
						Atom atom_if = new Atom();
						atom_if.var = _Var_if.getText();
						if(_Ind_if == null){
							
							atom_if.ind = null;	
						}else{
							atom_if.ind = _Ind_if.getText();
						}
						atom_if.rel = _Rel_if.getText();
						
						rule.or_and = Or_And.No_Or_And;
						rule.if_rule.add(atom_if);
						
					}else {
						List<Element> _Atoms_if = null;
						if(_And_if != null){
							
							rule.or_and = Or_And.And;
							_Atoms_if = _And_if.getChildren("Atom", ruleml);
						}
						if(_Or_if != null){
							
							rule.or_and = Or_And.Or;
							_Atoms_if = _Or_if.getChildren("Atom", ruleml);
						}
						
						
						for(Element _Atom_if: _Atoms_if){
							
							Element _Var_if = _Atom_if.getChild("Var", ruleml);
							Element _Ind_if = _Atom_if.getChild("Ind", ruleml);
							Element _op_if = _Atom_if.getChild("op", ruleml);
							Element _Rel_if = _op_if.getChild("Rel", ruleml);
							
							Atom atom_if = new Atom();
							atom_if.var = _Var_if.getText();
							if(_Ind_if == null){
								
								atom_if.ind = null;	
							}else{
								atom_if.ind = _Ind_if.getText();
							}
							
							atom_if.rel = _Rel_if.getText();
							
							rule.if_rule.add(atom_if);
						}
						
					}// end of if in ruleML
					///
					Element _then = _rule.getChild("then", ruleml);
	                Element _Atom_then = _then.getChild("Atom", ruleml);
					Element _Ind_then = _Atom_then.getChild("Ind", ruleml);
					Element _op_then = _Atom_then.getChild("op", ruleml);
					Element _Rel_then = _op_then.getChild("Rel", ruleml);
					
					Atom atom_then = new Atom();
					atom_then.ind = _Ind_then.getText();
					atom_then.rel = _Rel_then.getText();
					
					rule.then_rule.add(atom_then);
					
					//System.out.println("atom_then.ind:");
					//System.out.println(atom_then.ind);   
					rules.add(rule);
					
				}//end of for
			
			}catch(Exception e){
				
				e.printStackTrace();
			}
			
			return rules;
	    }
}
