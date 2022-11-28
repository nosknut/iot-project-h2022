const AWS = require('aws-sdk');

const dynamo = new AWS.DynamoDB.DocumentClient();

exports.handler = async (event, context) => {
    let body;
    let statusCode = '200';
    const headers = {
        'Content-Type': 'application/json',
    };

    try {
        switch (event.httpMethod) {
            case 'GET':
                body = await dynamo.query({
                    TableName: event.queryStringParameters.TableName,
                    Limit: event.queryStringParameters.Limit,
                    IndexName: event.queryStringParameters.IndexName,
                    ScanIndexForward: event.queryStringParameters.ascending,
                    KeyConditionExpression: "device_id = :s",
                    ExpressionAttributeValues: {
                        ":s": event.queryStringParameters.device_id,
                    },
                }).promise();
                break;
            default:
                throw new Error(`Unsupported method "${event.httpMethod}"`);
        }
    } catch (err) {
        statusCode = '400';
        body = err.message;
    } finally {
        body = JSON.stringify(body);
    }

    return {
        statusCode,
        body,
        headers: {
            ...headers,
            // TODO: Set proper CORS policy in API Gateway
            "Access-Control-Allow-Origin": "*",
        },
        
    };
};
