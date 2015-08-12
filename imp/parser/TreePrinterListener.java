import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Arrays;

public class TreePrinterListener implements ParseTreeListener {
    private final List<String> ruleNames;
    private final StringBuilder builder = new StringBuilder();
    private String sep = "";
    private boolean printMeta = false;

    public TreePrinterListener(Parser parser) {
        this.ruleNames = Arrays.asList(parser.getRuleNames());
    }

    public TreePrinterListener(List<String> ruleNames) {
        this.ruleNames = ruleNames;
    }

    @Override
    public void visitTerminal(TerminalNode node) {
        String name = getRuleName((ParserRuleContext)node.getParent());
        if (name.startsWith("raw_")) {
            // builder.append(sep);
            // Token symbol = (node.getSymbol().replace(")", "))"));
            Token symbol = (node.getSymbol());
            builder.append(symbol.getText());
            sep = ",";
        }
        // System.out.println(getRuleName((ParserRuleContext)node.getParent()));
        // Token symbol = (node.getSymbol();
        // if (symbol != null) {
        //     String s = symbol.getText();
        //     return s;
        // }
        // builder.append(Utils.escapeWhitespace(Trees.getNodeText(node, ruleNames), false));
        
    }

    @Override
    public void visitErrorNode(ErrorNode node) {
        builder.append(sep);

        builder.append(Utils.escapeWhitespace(Trees.getNodeText(node, ruleNames), false));
        sep = ",";
    }

    String getRuleName(ParserRuleContext ctx) {
        int ruleIndex = ctx.getRuleIndex();
        String ruleName;
        if (ruleIndex >= 0 && ruleIndex < ruleNames.size()) {
            ruleName = ruleNames.get(ruleIndex);
        } else {
            ruleName = Integer.toString(ruleIndex);
        }

        if (ruleName.lastIndexOf('_') == ruleName.length() - 1) {
            ruleName = ruleName.substring(0, ruleName.length() - 1);
        }
        if (ruleName.startsWith("raw_string")) {
            ruleName = "raw_string";
        } else if (ruleName.startsWith("raw_int")) {
            ruleName = "raw_int";
        } else if (ruleName.startsWith("raw_real")) {
            ruleName = "raw_real";
        } else if (ruleName.startsWith("raw_blob")) {
            ruleName = "raw_blob";
        }

        return ruleName;
    }

    @Override
    public void enterEveryRule(ParserRuleContext ctx) {
        builder.append(sep);

        // if (ctx.getChildCount() > 0) {
        //     builder.append('(');
        // }

        String ruleName = getRuleName(ctx);

        builder.append(ruleName);

        if (ctx.getChildCount() > 0) {
            builder.append('(');
        }
        sep = "";
    }

    // @Override
    // public void exitEveryRule(ParserRuleContext ctx) {
    //     if (ctx.getChildCount() > 0) {
    //         builder.append(')');
    //     }
    // }

    @Override
    public String toString() {
        return builder.toString();
    }

    @Override
    public void exitEveryRule(ParserRuleContext ctx) {
        if (ctx.getChildCount() > 0) {
            Token positionToken = ctx.getStart();
            if (positionToken != null) {
                if (printMeta) {
                    builder.append(" { line(");
                    builder.append(positionToken.getLine());
                    builder.append("), offsetStart(");
                    builder.append(positionToken.getStartIndex());
                    builder.append("), offsetEnd(");
                    builder.append(positionToken.getStopIndex());
                    builder.append(") }");
                }
                builder.append(")");
            }
            else {
                builder.append(')');
            }
        }
        sep = ",";
    }

    public static void main(String[] args) throws Exception {
        ImpLexer lexer = new ImpLexer(new ANTLRFileStream("prog1.imp"));
        ImpParser parser = new ImpParser(new CommonTokenStream(lexer));

        List<String> ruleNames = Arrays.asList(parser.getRuleNames());
        parser.setBuildParseTree(true);
        ParserRuleContext prc = parser.program();
        ParseTree tree = prc;

        TreePrinterListener listener = new TreePrinterListener(ruleNames);
        ParseTreeWalker.DEFAULT.walk(listener, tree);
        String formatted = listener.toString();
        System.out.println(formatted);

        // parser.setBuildParseTree(true);
        // ParserRuleContext prc = parser.program();
        // ParseTree t = prc.getChild(0);
        // ASTVisitor v = new ASTVisitor();
        // System.out.println("------------------------\ntoStringTree(parser)\n");
        // System.out.println(t.toStringTree(parser));
        // System.out.println("------------------------\ntoStringTree()\n");
        // System.out.println(t.toStringTree());
        // System.out.println("------------------------\nvisit\n");
        // String s = v.visit(t);

        // // ParserRuleContext t = parser.getRuleContext() ;
        // System.out.println("xxx\n" + s + "\nyyy\n");
    }
}

