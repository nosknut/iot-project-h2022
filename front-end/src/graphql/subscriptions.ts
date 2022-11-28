/* tslint:disable */
/* eslint-disable */
// this is an auto generated file. This will be overwritten

export const onCreateDevice = /* GraphQL */ `
  subscription OnCreateDevice(
    $filter: ModelSubscriptionDeviceFilterInput
    $owner: String
  ) {
    onCreateDevice(filter: $filter, owner: $owner) {
      id
      deviceId
      name
      type
      createdAt
      updatedAt
      owner
    }
  }
`;
export const onUpdateDevice = /* GraphQL */ `
  subscription OnUpdateDevice(
    $filter: ModelSubscriptionDeviceFilterInput
    $owner: String
  ) {
    onUpdateDevice(filter: $filter, owner: $owner) {
      id
      deviceId
      name
      type
      createdAt
      updatedAt
      owner
    }
  }
`;
export const onDeleteDevice = /* GraphQL */ `
  subscription OnDeleteDevice(
    $filter: ModelSubscriptionDeviceFilterInput
    $owner: String
  ) {
    onDeleteDevice(filter: $filter, owner: $owner) {
      id
      deviceId
      name
      type
      createdAt
      updatedAt
      owner
    }
  }
`;
