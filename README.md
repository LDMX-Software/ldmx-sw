Tracking
---------

An `ldmx-sw` submodule focused on the digitization of tracker hits and track finding/fitting using ACTS.

#### Development

Developing within this module requires cloning of `ldmx-sw` as outlined in the 
[Quick Start](https://github.com/LDMX-Software/ldmx-sw#quick-start).  Once cloned, 
a development branch can be created as follows
```bash
cd ldmx-sw/Tracking
git checkout -b trk_dev_iss<#> 
git push origin trk_dev_iss<#>
```
Where `<#>` represents the number of the associated issue.  After a branch has been created, developments 
can be built using the instructions in the Quick Start pointed to above.

Once developments have been tested and merged into the `main` tracking branch, the `trunk` of `ldmx-sw` 
needs to be updated to point to the new `main` hash.  This done by creating a branch of `ldmx-sw`'s 
trunk, committing the update `main` of the `Tracking` module and opening a pull request into `ldmx-sw`. 

#### Track Reconstruction

## Contributors

<a href="https://github.com/LDMX-Software/Tracking/graphs/contributors">
  <img src="https://contributors-img.web.app/image?repo=LDMX-Software/Tracking" />
</a>



