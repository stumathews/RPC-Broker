


/* -----------------------------
 * Broker Protocol specification
 * ------------------------------

    {type: iut} 0|1
    {id: int}
    {header_n:value_n}
    {header_n+1:value_n+1}
    {header...:value...}
    {sender: ip|dns" }

ServiceRegistration:

    {services: ["service1", "service2","service3"]}
    {servicename: "ServiceId"}

ServiceRequest:

   {"op"=>"getServerDate"}
   {"params" => [ buffer,length ]}

ServiceRegistrationReply:

    ack_result: int

ServiceRequestReply:

    
    { op=>"getServerDate"}
    { response => [data1, data2] }

*/
