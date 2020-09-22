alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'

alias ls='ls -FGhp'
alias lr='ls -R | grep ":$" | sed -e '\''s/:$//'\'' -e '\''s/[^-][^\/]*\//—/g'\'' -e '\''s/^/ /'\'' -e '\''s/-/|/'\'' | less'
export LSCOLORS=GxFxCxDxBxegedabagaced


export GOPATH=$HOME/go
export GOROOT="$(brew --prefix golang)/libexec"
export PATH=$PATH:$GOPATH/bin:$GOROOT/bin:$HOME/workspace/hyperledger/fabric-samples/bin


#export PATH=$PATH:$GOPATH/bin:$HOME/workspace/hyperledger/fabric-samples/bin:$GOPATH/src/github.com/hyperledger/fabric/build/bin
