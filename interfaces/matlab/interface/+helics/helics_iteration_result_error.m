function v = helics_iteration_result_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 114);
  end
  v = vInitialized;
end
