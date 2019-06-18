package inferEngine;

import java.util.ArrayList;
import java.util.List;

interface Or_And{   
    int Or = 0;   
    int And = 1;   
    int No_Or_And = 2;      
} 

public class RuleML {

	public String ruleName;
	public int or_and;
	public List<Atom> if_rule = new ArrayList<Atom>();
	public List<Atom> then_rule = new ArrayList<Atom>();
}
