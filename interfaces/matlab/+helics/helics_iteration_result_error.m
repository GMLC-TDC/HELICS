function v = helics_iteration_result_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183118);
  end
  v = vInitialized;
end
