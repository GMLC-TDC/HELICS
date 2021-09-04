function v = helics_iteration_result_next_step()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 128);
  end
  v = vInitialized;
end
