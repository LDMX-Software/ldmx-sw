
I am updating _ldmx-sw_, here are the details.

### What are the issues that this addresses?
_Hint_: Use the phrase '_This resolves #< issue number >_' so that they are linked automatically.

## Check List
- [ ] I successfully compiled _ldmx-sw_ with my developments

- [ ] I ran my developments and the following shows that they are successful.

_< put plots or some other proof that your developments work >_

- [ ] I attached any sub-module related changes to this PR.

_Explanation_: You need to specifically `git add` your submodule's commit in _ldmx-sw_ and then have a separate PR within that submodule.
```
# outline of submodule commit updating
cd SUBMODULE
git checkout updates
cd ..
git add SUBMODULE
git commit -m "I updated the commit for SUBMODULE to the last commit of my-updates"
```

### Related Sub-Module PRs
If you have developments in a submodule that you need to merge in along with these developments, please link those PRs here, otherwise just delete this section.
_Note_: You do not need to make a Sub-Module PR if your submodule developments are already on that repo's default branch.

- 
