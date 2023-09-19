import argparse
import sys

class ArgParser(argparse.ArgumentParser):
    def __init__(self):
        super(ArgParser, self).__init__()
        self.add_argument('-f', '--from', type=str, default='from',
                          help='The path of the input file containing AMIE+ rules')
        self.add_argument('-t', '--to', type=str, default='to',
                          help='The path of the output file containing transformed rules')
        self.add_argument('--pca', action='store_true',
                          help='Use pca confidence instead of std')
    def parse_args(self):
        args = super().parse_args()
        return args

def read_file(path):
    content = None
    with open(path, encoding="ansi") as infile:
        content = infile.readlines()
    content = [x.strip() for x in content]
    return content
    
def rewriteRule(amie_rule):
    variables = ["A", "B", "C", "D", "E"]
    amie_rule = amie_rule.replace("?a", "X")
    amie_rule = amie_rule.replace("?b", "Y")
    
    for i in range(ord('a'), ord('z')+1):
        if "?" + chr(i) in amie_rule:
            amie_rule = amie_rule.replace("?" + chr(i), variables.pop(0))
    
    anyBURL = ""
    body,_,head = amie_rule.partition("   => ")
    
    head = head.split("  ")
    anyBURL = anyBURL + f"{head[1]}({head[0]},{head[2]}) <= "
    
    body = body.split("  ")
    bodies = []
    for i in range(0,len(body),3):
        h,r,t = body[i:i+3]
        bodies.append(f"{r}({h},{t})")
    if "X)" in bodies[-1] or "(X" in bodies[-1]:
        bodies = reversed(bodies)
    anyBURL = anyBURL + ", ".join(bodies)
    
    return anyBURL
    
def amieToAnyBURL(in_path, out_path, pca):
    content = read_file(rules)

    AnyBURL = []
    for rule in content:
        #Std. Lower Bound	PCA Lower Bound	PCA Conf estimation
        Rule, Head_Coverage, Std_Confidence, PCA_Confidence, Positive_Examples, Body_size, PCA_Body_size, Functional_variable,*_ = rule.split("\t")
        
        if pca:
            AnyBURL.append(PCA_Body_size.replace(",",".") + "\t" + Positive_Examples.replace(",",".") + "\t" + PCA_Confidence.replace(",",".") + "\t" + rewriteRule(Rule))
        else:
            AnyBURL.append(Body_size.replace(",",".") + "\t" + Positive_Examples.replace(",",".") + "\t" + Std_Confidence.replace(",",".") + "\t" + rewriteRule(Rule))
            
    with open(rules_out, "w") as outfile:
            for rule in AnyBURL:
                outfile.write(rule + "\n")
    


if __name__ == "__main__":
    args = ArgParser().parse_args()
    amieToAnyBURL(getattr(args,'from'), getattr(args,'to'), getattr(args,'pca'))
