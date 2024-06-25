import { ComponentFixture, TestBed } from '@angular/core/testing';

import { LdmxExperimentComponent } from './ldmx-experiment.component';

describe('LdmxExperimentComponent', () => {
  let component: LdmxExperimentComponent;
  let fixture: ComponentFixture<LdmxExperimentComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [LdmxExperimentComponent]
    })
    .compileComponents();

    fixture = TestBed.createComponent(LdmxExperimentComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
