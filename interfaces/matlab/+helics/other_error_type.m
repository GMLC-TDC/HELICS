function v = other_error_type()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183079);
  end
  v = vInitialized;
end
