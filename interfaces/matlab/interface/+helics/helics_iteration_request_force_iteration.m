function v = helics_iteration_request_force_iteration()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 97);
  end
  v = vInitialized;
end
