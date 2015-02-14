import java.util.*;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;
import org.antlr.v4.runtime.Parser.TraceListener;
import org.antlr.v4.runtime.misc.NotNull;
import java.lang.Exception;


public class ASTVisitor extends AbstractParseTreeVisitor<String> {
	public String visit(@NotNull ParseTree t) {
		String s = "";
		if (t instanceof RuleNode) {
			String name = getName((RuleNode)t);
			String children = visitChildren((RuleNode)t);
			s = "(" + name + " " + children + ")";
		} else if (t instanceof ErrorNode) {
			System.out.println("t is an ErrorNode");
			s = visitErrorNode((ErrorNode)t);
		} else if (t instanceof TerminalNode) {
			s = visitTerminal((TerminalNode)t);
		}
		return s;
	}
	public String getName(RuleNode t) {
		String classname = t.getClass().getName();
		String withoutBase = classname.substring(classname.indexOf("$") + 1);
		String name = withoutBase.substring(0, withoutBase.indexOf("Context"));
		return name;
	}

	public String visitChildren(@NotNull RuleNode node) {
		String results = "";
		for (int i = 0; i < node.getChildCount(); i++) {
			String newResult = visit(node.getChild(i));
			if (results.length() == 0) {
				results = newResult;
			} else {
				results = results + " " + newResult;
			}
		}

		return results;
	}
	public String visitTerminal(TerminalNode node) {
		// System.out.println("Visiting " + node.getClass().getName());
		return "(Terminal " + node.getText() + ")";
	}

	protected String aggregateResult(String aggregate, String nextResult) {
		return aggregate + " " + nextResult;
	}
    public static void main(String[] args) throws Exception {
        ImpUglyLexer lexer = new ImpUglyLexer(new ANTLRFileStream("prog1.imp"));
        ImpUglyParser parser = new ImpUglyParser(new CommonTokenStream(lexer));
        parser.setBuildParseTree(true);
        ParserRuleContext prc = parser.program();
        ParseTree t = prc.getChild(0);
        ASTVisitor v = new ASTVisitor();
        System.out.println("------------------------\ntoStringTree(parser)\n");
        System.out.println(t.toStringTree(parser));
        System.out.println("------------------------\ntoStringTree()\n");
        System.out.println(t.toStringTree());
        System.out.println("------------------------\nvisit\n");
        String s = v.visit(t);

        // ParserRuleContext t = parser.getRuleContext() ;
        System.out.println("xxx\n" + s + "\nyyy\n");
    }
}