package com.amazonaws.lambda.trash;

import com.amazonaws.auth.AWSStaticCredentialsProvider;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.regions.Regions;
import com.amazonaws.services.lambda.runtime.Context;
import com.amazonaws.services.lambda.runtime.RequestHandler;
import com.amazonaws.services.sns.AmazonSNS;
import com.amazonaws.services.sns.AmazonSNSClientBuilder;
import com.amazonaws.services.sns.model.PublishRequest;
import com.amazonaws.services.sns.model.PublishResult;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

public class Trash implements RequestHandler<Object, String> {

	@Override
	public	String	handleRequest(Object	input,	Context	context) {
					context.getLogger().log("Input:	" +	input);
					String	json	= ""+input;
					JsonParser	parser	= new JsonParser();
					JsonElement	element	=	parser.parse(json);
					JsonElement	state	=	element.getAsJsonObject().get("state");
					JsonElement	reported	=	state.getAsJsonObject().get("reported");
					String	distance	=	reported.getAsJsonObject().get("distance").getAsString();
					double	dist	=	Double.valueOf(distance);
					final	String	AccessKey="AKIAQ2UB6XQITBY2VG56";
					final	String	SecretKey="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";
					final	String	topicArn="arn:aws:sns:ap-northeast-2:057180863505:Ultrasonic";
					BasicAWSCredentials	awsCreds	= new BasicAWSCredentials(AccessKey,	SecretKey);		
					AmazonSNS	sns	=	AmazonSNSClientBuilder.standard()
																	.withRegion(Regions.AP_NORTHEAST_2)
																	.withCredentials( new AWSStaticCredentialsProvider(awsCreds) )
																	.build();
					final	String	msg	= "Full";
					final	String	subject	= "Critical	Warning";
					if ( dist	< 5) {
									PublishRequest	publishRequest	= new PublishRequest(topicArn,	msg,	subject);
									PublishResult	publishResponse	=	sns.publish(publishRequest);
					}
					return	subject+"distance = " + distance;
	}

}
